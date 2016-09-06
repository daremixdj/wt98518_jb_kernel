/* MPU6050C_ACC motion sensor driver
 *
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>

#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "mpu60x0.h"
#include "mpu6050c.h"
#include <linux/hwmsen_helper.h>
#include <linux/kernel.h>

#include <linux/hardware_info.h>

//#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_boot.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE

/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_MPU6050C_ACC	3000
/*----------------------------------------------------------------------------*/
#define MPU6050C_DEFAULT_FS		MPU6050C_FS_1000
#define MPU6050C_DEFAULT_LSB		MPU6050C_FS_1000_LSB
/*---------------------------------------------------------------------------*/
#define DEBUG 0
/*----------------------------------------------------------------------------*/
#define CONFIG_MPU6050C_ACC_LOWPASS   /*apply low pass filter on output*/       
/*----------------------------------------------------------------------------*/
#define MPU6050C_ACC_AXIS_X          0
#define MPU6050C_ACC_AXIS_Y          1
#define MPU6050C_ACC_AXIS_Z          2
#define MPU6050C_ACC_AXES_NUM        3
#define MPU6050C_ACC_DATA_LEN        6   
#define MPU6050C_ACC_DEV_NAME        "MPU6050C_ACC"
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id mpu6050c_acc_i2c_id[] = {{MPU6050C_ACC_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_mpu6050c_acc={ I2C_BOARD_INFO("MPU6050C_ACC", MPU6050C_ACC_I2C_SLAVE_ADDR>>1)};
/*the adapter id will be available in customization*/
//static unsigned short mpu6050c_acc_force[] = {0x00, MPU6050C_ACC_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const mpu6050c_acc_forces[] = { mpu6050c_acc_force, NULL };
//static struct i2c_client_address_data mpu6050c_acc_addr_data = { .forces = mpu6050c_acc_forces,};

static int packet_thresh = 75; // 600 ms / 8ms/sample
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_remap_to_gravity(struct i2c_client *client, int data[MPU6050C_ACC_AXES_NUM]);

struct GS_CALI_DATA_STRUCT
{
    int x;
    int y;
    int z;
	int valid;
};
static struct GS_CALI_DATA_STRUCT gs_cali={0,0,0,0};
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int mpu6050c_acc_i2c_remove(struct i2c_client *client);
//static int mpu6050c_acc_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
//static int mpu6050c_acc_suspend(struct i2c_client *client, pm_message_t msg) ;
//static int mpu6050c_acc_resume(struct i2c_client *client);
/*----------------------------------------------------------------------------*/
typedef enum {
    ACC_TRC_FILTER  = 0x01,
    ACC_TRC_RAWDATA = 0x02,
    ACC_TRC_IOCTL   = 0x04,
    ACC_TRC_CALI	= 0X08,
    ACC_TRC_INFO	= 0X10,
    ACC_TRC_DATA	= 0X20,
} ACC_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][MPU6050C_ACC_AXES_NUM];
    int sum[MPU6050C_ACC_AXES_NUM];
    int num;
    int idx;
};

typedef enum {
    CMC_BIT_ACC    = 1,
    CMC_BIT_GYRO     = 2,
} CMC_BIT;

/*----------------------------------------------------------------------------*/
struct mpu6050c_acc_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
    
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
    atomic_t				filter;
    s16                     acc_cali_sw[MPU6050C_ACC_AXES_NUM+1];
    s16                     gyro_cali_sw[MPU6050C_ACC_AXES_NUM+1];

    /*data*/
    s8                      offset[MPU6050C_ACC_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[MPU6050C_ACC_AXES_NUM+1];
	uint32_t valid;
	s8	mpu_chipid;

#if defined(CONFIG_MPU6050C_ACC_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif     
	ulong		enable; 		/*enable mask*/

};
/*----------------------------------------------------------------------------*/
static struct i2c_driver mpu6050c_acc_i2c_driver = {
    .driver = {
        .name           = MPU6050C_ACC_DEV_NAME,
    },
	.probe      		= mpu6050c_acc_i2c_probe,
	.remove    			= mpu6050c_acc_i2c_remove,
#if !defined(CONFIG_HAS_EARLYSUSPEND)    
    .suspend            = mpu6050c_acc_suspend,
    .resume             = mpu6050c_acc_resume,
#endif
	.id_table = mpu6050c_acc_i2c_id,
};

/*----------------------------------------------------------------------------*/
struct i2c_client *mpu6050c_acc_i2c_client = NULL;
static struct platform_driver mpu6050c_acc_driver;
struct mpu6050c_acc_i2c_data *mpu6050c_obj_i2c_data = NULL;
static int mpu6050c_set_powermode(struct i2c_client *client,int value);



/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
/*

//----------------------------------------------------------------------------//
static struct data_resolution mpu6050c_acc_offset_resolution = {{15, 6}, 64};
*/
/*--------------------accscopy power control function----------------------------------*/
static struct data_resolution mpu6050c_data_resolution[] = {
 /* combination by {FULL_RES,RANGE}*/
	 {{ 1, 0}, 16384},   // dataformat +/-2g	in 16-bit resolution;    16384 = (2^16)/(2*2)			
};

/*----------------------------------------------------------------------------*/


struct mpu6050c_acc_i2c_data* MPU6050C_Acc_GetI2CData()
{
	return mpu6050c_obj_i2c_data;
}
EXPORT_SYMBOL_GPL(MPU6050C_Acc_GetI2CData);


static void MPU6050C_ACC_power(struct acc_hw *hw, unsigned int on) 
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "MPU6050C_ACC"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "MPU6050C_ACC"))
			{
				GSE_ERR("power off fail!!\n");
			}			  
		}
	}
	power_on = on;    
}
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_write_rel_calibration(struct mpu6050c_acc_i2c_data *obj, int dat[MPU6050C_ACC_AXES_NUM])
{
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_X] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_X]];
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_Y] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Y]];
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_Z] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Z]];
#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("test  (%5d, %5d, %5d) ->(%5d, %5d, %5d)->(%5d, %5d, %5d))\n", 
				obj->cvt.sign[MPU6050C_ACC_AXIS_X],obj->cvt.sign[MPU6050C_ACC_AXIS_Y],obj->cvt.sign[MPU6050C_ACC_AXIS_Z],
				dat[MPU6050C_ACC_AXIS_X], dat[MPU6050C_ACC_AXIS_Y], dat[MPU6050C_ACC_AXIS_Z],
				obj->cvt.map[MPU6050C_ACC_AXIS_X],obj->cvt.map[MPU6050C_ACC_AXIS_Y],obj->cvt.map[MPU6050C_ACC_AXIS_Z]);
			GSE_LOG("write acc calibration data  (%5d, %5d, %5d)\n", 
				obj->acc_cali_sw[MPU6050C_ACC_AXIS_X],obj->acc_cali_sw[MPU6050C_ACC_AXIS_Y],obj->acc_cali_sw[MPU6050C_ACC_AXIS_Z]);
		}
#endif
    return 0;
}
static int MPU6050C_ACC_write_gyro_rel_calibration(struct mpu6050c_acc_i2c_data *obj, int dat[MPU6050C_ACC_AXES_NUM])
{
    obj->gyro_cali_sw[MPU6050C_ACC_AXIS_X] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_X]];
    obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Y] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Y]];
    obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Z] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Z]];
#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("test  (%5d, %5d, %5d) ->(%5d, %5d, %5d)->(%5d, %5d, %5d))\n", 
				obj->cvt.sign[MPU6050C_ACC_AXIS_X],obj->cvt.sign[MPU6050C_ACC_AXIS_Y],obj->cvt.sign[MPU6050C_ACC_AXIS_Z],
				dat[MPU6050C_ACC_AXIS_X], dat[MPU6050C_ACC_AXIS_Y], dat[MPU6050C_ACC_AXIS_Z],
				obj->cvt.map[MPU6050C_ACC_AXIS_X],obj->cvt.map[MPU6050C_ACC_AXIS_Y],obj->cvt.map[MPU6050C_ACC_AXIS_Z]);
			GSE_LOG("write acc calibration data  (%5d, %5d, %5d)\n", 
				obj->gyro_cali_sw[MPU6050C_ACC_AXIS_X],obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Y],obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Z]);
		}
#endif
    return 0;
}



/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ResetCalibration(struct i2c_client *client)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);	

	memset(obj->acc_cali_sw, 0x00, sizeof(obj->acc_cali_sw));
	obj->valid=0;
	
	return 0;    
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ResetGyroCalibration(struct i2c_client *client)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);	

	memset(obj->gyro_cali_sw, 0x00, sizeof(obj->gyro_cali_sw));
	return 0;    
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ReadCalibration(struct i2c_client *client, int dat[MPU6050C_ACC_AXES_NUM])
{
    struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);

    dat[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_X];
    dat[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_Y];
    dat[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_Z];

#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("Read acc calibration data  (%5d, %5d, %5d)\n", 
				dat[MPU6050C_ACC_AXIS_X],dat[MPU6050C_ACC_AXIS_Y],dat[MPU6050C_ACC_AXIS_Z]);
		}
#endif
                                       
    return 0;
}
/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ReadGyroCalibration(struct i2c_client *client, int dat[MPU6050C_ACC_AXES_NUM])
{
    struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);

    dat[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_X];
    dat[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Y];
    dat[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Z];

#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("Read acc calibration data  (%5d, %5d, %5d)\n", 
				dat[MPU6050C_ACC_AXIS_X],dat[MPU6050C_ACC_AXIS_Y],dat[MPU6050C_ACC_AXIS_Z]);
		}
#endif
                                       
    return 0;
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_WriteAccCalibration_New(struct i2c_client *client, struct GS_CALI_DATA_STRUCT *data_cali)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);

	GSE_FUN();
	if(!obj || !data_cali)
	{
		GSE_ERR("null ptr!!\n");
		return -EINVAL;
	}
	else
	{ 
		if(data_cali->valid == 1)
		{
			gs_cali.x = data_cali->x;
			gs_cali.y = data_cali->y;
			gs_cali.z = data_cali->z;
			gs_cali.valid = data_cali->valid;
		}
		else
		{
			gs_cali.x = 0;
			gs_cali.y = 0;
			gs_cali.z = 0;
			gs_cali.valid = 0;
		}
	} 
	return 0;
}
/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_WriteCalibration(struct i2c_client *client, int dat[MPU6050C_ACC_AXES_NUM])
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
	int cali[MPU6050C_ACC_AXES_NUM];
	#if 0


	GSE_FUN();
	if(!obj || ! dat)
	{
		GSE_ERR("null ptr!!\n");
		return -EINVAL;
	}
	else
	{        		
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_X];
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_Y];
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->acc_cali_sw[MPU6050C_ACC_AXIS_Z]; 
		cali[MPU6050C_ACC_AXIS_X] += dat[MPU6050C_ACC_AXIS_X];
		cali[MPU6050C_ACC_AXIS_Y] += dat[MPU6050C_ACC_AXIS_Y];
		cali[MPU6050C_ACC_AXIS_Z] += dat[MPU6050C_ACC_AXIS_Z];
#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("write acc calibration data  (%5d, %5d, %5d)-->(%5d, %5d, %5d)\n", 
				dat[MPU6050C_ACC_AXIS_X], dat[MPU6050C_ACC_AXIS_Y], dat[MPU6050C_ACC_AXIS_Z],
				cali[MPU6050C_ACC_AXIS_X],cali[MPU6050C_ACC_AXIS_Y],cali[MPU6050C_ACC_AXIS_Z]);
		}
#endif
		return MPU6050C_ACC_write_rel_calibration(obj, cali);
	} 
#endif 
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_X] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_X]];
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_Y] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Y]];
    obj->acc_cali_sw[MPU6050C_ACC_AXIS_Z] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*dat[obj->cvt.map[MPU6050C_ACC_AXIS_Z]];

	return err;
}
static int MPU6050C_ACC_WriteGyroCalibration(struct i2c_client *client, int dat[MPU6050C_ACC_AXES_NUM])
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
	int cali[MPU6050C_ACC_AXES_NUM];


	GSE_FUN();
	if(!obj || ! dat)
	{
		GSE_ERR("null ptr!!\n");
		return -EINVAL;
	}
	else
	{        		
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_X];
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Y];
		cali[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Z]; 
		cali[MPU6050C_ACC_AXIS_X] += dat[MPU6050C_ACC_AXIS_X];
		cali[MPU6050C_ACC_AXIS_Y] += dat[MPU6050C_ACC_AXIS_Y];
		cali[MPU6050C_ACC_AXIS_Z] += dat[MPU6050C_ACC_AXIS_Z];
#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_CALI)
		{
			GSE_LOG("write acc calibration data  (%5d, %5d, %5d)-->(%5d, %5d, %5d)\n", 
				dat[MPU6050C_ACC_AXIS_X], dat[MPU6050C_ACC_AXIS_Y], dat[MPU6050C_ACC_AXIS_Z],
				cali[MPU6050C_ACC_AXIS_X],cali[MPU6050C_ACC_AXIS_Y],cali[MPU6050C_ACC_AXIS_Z]);
		}
#endif
		return MPU6050C_ACC_write_gyro_rel_calibration(obj, cali);
	} 

	return err;
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_Get_ChipId(struct i2c_client *client)
{
	int res = 0;
	u8  chip_id = 0;
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	
	GSE_LOG("Get MPU ChipId\n");
	res = hwmsen_read_byte(client,MPU6050C_ACC_REG_DEVICE_ID, &chip_id);
	if (res) 
	{
		GSE_ERR("MPU6050C Get_ChipId error,res: %d!\n", res);
		return res;
	}
	GSE_LOG("get Chip ID is :%d\n",chip_id);
	switch(chip_id){
		case MPU6050C_CHIP_ID:
			obj->mpu_chipid = MPU6050C_CHIP_ID;
			break;
		case MPU6880_CHIP_ID:
			obj->mpu_chipid = MPU6880_CHIP_ID;
			break;
		default:
			obj->mpu_chipid = MPU_INVALID_ID;
			break;			
	}
	return 0;
}
/*----------------------------------------------------------------------------*/

static int MPU6050C_ACC_Reset(struct i2c_client *client)
{
	int res = 0;
	GSE_LOG("Reset MPU6050 B1\n");
	res = hwmsen_write_byte(client,MPU6050C_ACC_REG_PWR_MGMT_1,BIT_HW_RESET);
	if (res) 
	{
		GSE_ERR(" Reset MPU6050 B1 error,res: %d!\n", res);
		return res;
	}
	msleep(40);
	return 0;
}
/*----------------------------------------------------------------------------*/

static int MPU6050C_ACC_SetPWR_MGMT_12(struct i2c_client *client)
{
	u8 databuf1[2] = {0};
	u8 databuf2[2] = {0};
	int res = 0;
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	//GSE_FUN();

	if((!test_bit(CMC_BIT_GYRO, &obj->enable))
		&&(test_bit(CMC_BIT_ACC, &obj->enable)))
	{ 
		//means it works only for acc,so set gyro as standby mode
		GSE_LOG("MPU6050C_ACC_SetPWR_MGMT_12, acc works only, !CMC_BIT_GYRO&&CMC_BIT_ACC \n");

	if(obj->mpu_chipid == MPU6050C_CHIP_ID){		//MPU6050 
		databuf1[0] = 0x28;  // CYCLE =1 ;SLEEP =0; TEMP_DIS =1 Internel_8MHz

		databuf2[0] = 0xC7;	 // STBY_XG =1,STBY_YG =1,STBY_ZG =1 
	}
	else if(obj->mpu_chipid == MPU6880_CHIP_ID){	//MPU6880
	
		databuf2[0] = MPU6050C_ACC_REG_PWR_MGMT_2;
		databuf2[1] = 0x07;
		res = i2c_master_send(client, databuf2, 0x2);
		if(res <= 0)
		{
			GSE_LOG("set power mode failed! 0x6C\n");
			return MPU6050C_ACC_ERR_I2C;
		}
		databuf2[0] = MPU6880C_ACC_REG_LP_ACCEL_ODR;
		databuf2[1] = 0x07;
		res = i2c_master_send(client, databuf2, 0x2);
		if(res <= 0)
		{
			GSE_LOG("set power mode failed! 0x1E\n");
			return MPU6050C_ACC_ERR_I2C;
		}
		databuf2[0] = MPU6050C_ACC_REG_PWR_MGMT_1;
		databuf2[1] = 0x28;
		res = i2c_master_send(client, databuf2, 0x2);
		if(res <= 0)
		{
			GSE_LOG("set power mode failed! 0x6B\n");
			return MPU6050C_ACC_ERR_I2C;
		}
		return MPU6050C_ACC_SUCCESS;
	  }
	}
	else if((test_bit(CMC_BIT_GYRO, &obj->enable))
		&&(test_bit(CMC_BIT_ACC, &obj->enable)))
	{
		//mean  work both
		GSE_LOG("MPU6050C_ACC_SetPWR_MGMT_12, work both, CMC_BIT_GYR&&CMC_BIT_ACC \n");
		databuf1[0] = 0x01;//SLEEP=0;CLKSEL =1 MPU6050C_ACC_CLKSEL_PLL_X

		databuf2[0] = 0x00;
	}
	else if((test_bit(CMC_BIT_GYRO, &obj->enable))
		&&(!test_bit(CMC_BIT_ACC, &obj->enable)))
	{
		//mean	gyro work only,acc sleep
		GSE_LOG("MPU6050C_ACC_SetPWR_MGMT_12, gyro work only, CMC_BIT_GYR&&!CMC_BIT_ACC \n");
		databuf1[0] = 0x01;//SLEEP=0;CLKSEL =1 MPU6050C_ACC_CLKSEL_PLL_X

		databuf2[0] = 0x38;//STBY_XA =1,STBY_YA =1,STBY_ZA =1
	}
	else if((!test_bit(CMC_BIT_GYRO, &obj->enable))
		&&(!test_bit(CMC_BIT_ACC, &obj->enable)))
	{
		//mean both sleep
		GSE_LOG("MPU6050C_ACC_SetPWR_MGMT_12, both sleep,!CMC_BIT_GYR&&!CMC_BIT_ACC \n");
		databuf1[0] |= MPU6050C_ACC_SLEEP;
		databuf1[0] |= MPU6050C_ACC_CLKSEL_PLL_X;

		databuf2[0] = 0x00;
		
	}
	
	databuf1[1] = databuf1[0];
	databuf1[0] = MPU6050C_ACC_REG_PWR_MGMT_1;  
	res = i2c_master_send(client, databuf1, 0x2);
	if(res <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return MPU6050C_ACC_ERR_I2C;
	}

	databuf2[1] = databuf2[0];
	databuf2[0] = MPU6050C_ACC_REG_PWR_MGMT_2;
	res = i2c_master_send(client, databuf2, 0x2);
	if(res <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	return MPU6050C_ACC_SUCCESS;
}

/*----------------------------------------------------------------------------*/
static int MPU6880_SetDataBandWidth_Accel(struct i2c_client *client, u8 BandWidth)
{
	u8 databuf[2];	  
	int res = 0;

	memset(databuf, 0, sizeof(u8)*2);	 
	databuf[0] = MPU6880C_ACC_REG_CONFIG2;    
	databuf[1] = BandWidth&0x0F;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return MPU6050C_ACC_ERR_I2C;
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_SetAccDataRange(struct i2c_client *client, u8 range)
{
	
	u8 databuf[2];	  
	int res = 0;

	memset(databuf, 0, sizeof(u8)*2);	 
	databuf[0] = MPU6050C_ACC_REG_ACC_CONFIG;    
	databuf[1] = range;
	databuf[1] = databuf[1]<<3;//refer to datasheet
	databuf[1] &= 0x18;
	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MPU6050C_ACC_ERR_I2C;
	}

	mpu6050c_obj_i2c_data->reso = &mpu6050c_data_resolution[0];
	return 0;

}
static int MPU6050C_ACC_SetGyroDataRange(struct i2c_client *client, u8 range)
{
	
	u8 databuf[2];	  
	int res = 0;

	memset(databuf, 0, sizeof(u8)*2);	 
	databuf[0] = MPU6050C_ACC_REG_GYRO_CONFIG;    
	databuf[1] = range;
	databuf[1] = databuf[1]<<3;//refer to datasheet
	databuf[1] &= 0x18;


	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MPU6050C_ACC_ERR_I2C;
	}

	return 0;

}

static int MPU6050C_ACC_SetDataBandWidth(struct i2c_client *client, u8 BandWidth)
{
	
	u8 databuf[2];	  
	int res = 0;

	memset(databuf, 0, sizeof(u8)*2);	 
	databuf[0] = MPU6050C_ACC_REG_DATA_CONFIG;    
	databuf[1] = BandWidth&0x07;

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return MPU6050C_ACC_ERR_I2C;
	}

	return 0;

}

// set the sample rate
static int MPU6050C_ACC_SetSampleRate(struct i2c_client *client, int sample_rate)
{
	u8 databuf[2] = {0}; 
	int smplrt_div = 0;
	int res = 0;
	GSE_FUN();    

	if(hwmsen_read_byte(client, MPU6050C_ACC_REG_DATA_CONFIG, databuf))
	{
		GSE_ERR("read acc data format register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	else
	{
		GSE_LOG("read  acc data format register: 0x%x\n", databuf[0]);
	}

	if((databuf[0] & 0x07) == 0)	//Analog sample rate is 8KHz
	{
		smplrt_div = 8 * 1000 / sample_rate - 1;
	}
	else	// 1kHz
	{
		smplrt_div = 1000 / sample_rate - 1;
	}

	if(smplrt_div > 255)	// rate_div: 0 to 255;
	{
		smplrt_div = 255;
	}
	else if(smplrt_div < 0)
	{
		smplrt_div = 0;
	}
	
	databuf[0] = MPU6050C_ACC_REG_DATA_SMPRT_DIV;
	databuf[1] = smplrt_div;
	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		GSE_ERR("write sample rate register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}

	//read sample div after written for test	
	udelay(500);
	if(hwmsen_read_byte(client, MPU6050C_ACC_REG_DATA_SMPRT_DIV, databuf))
	{
		GSE_ERR("read acc sample rate register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	else
	{
		GSE_LOG("read  acc sample rate: 0x%x\n", databuf[0]);
	}
	
	return MPU6050C_ACC_SUCCESS;    
}


static int MPU6050_ACC_ReadStart(struct i2c_client *client, bool enable)
{
	u8 databuf[2] = {0};	
	int res = 0;
	GSE_FUN();    

	databuf[0] = MPU6050C_ACC_REG_FIFO_EN;  

	if (enable)
	{
		//enable xyz gyro in FIFO
		databuf[1] = (MPU6050C_ACC_FIFO_GYROX_EN|MPU6050C_ACC_FIFO_GYROY_EN|MPU6050C_ACC_FIFO_GYROZ_EN);
	}
	else
	{
		//disable xyz gyro in FIFO
		databuf[1] = 0;
	}

	res = i2c_master_send(client, databuf, 0x2);
	if (res <= 0)
	{
		GSE_ERR(" enable xyz gyro in FIFO error,enable: 0x%x!\n", databuf[1]);
		return MPU6050C_ACC_ERR_I2C;
	}
	GSE_LOG("MPU6050_ACC_ReadStart: enable xyz gyro in FIFO: 0x%x\n", databuf[1]);
	return MPU6050C_ACC_SUCCESS;
}

static int MPU6050_ACC_FIFOConfig(struct i2c_client *client, u8 clk)
{
	u8 databuf[2] = {0};	
	int res = 0;
	GSE_FUN();    

	//use gyro X, Y or Z for clocking
	databuf[0] = MPU6050C_ACC_REG_PWR_MGMT_1;	 
	databuf[1] = clk;
	
	res = i2c_master_send(client, databuf, 0x2);
	if (res <= 0)
	{
		GSE_ERR("write Power CTRL register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	GSE_LOG("MPU6050 use gyro X for clocking OK!\n");

	mdelay(100);

	//enable xyz gyro in FIFO
	databuf[0] = MPU6050C_ACC_REG_FIFO_EN;
	databuf[1] = (MPU6050C_ACC_FIFO_GYROX_EN|MPU6050C_ACC_FIFO_GYROY_EN|MPU6050C_ACC_FIFO_GYROZ_EN);

	res = i2c_master_send(client, databuf, 0x2);
	if (res <= 0)
	{
		GSE_ERR("write Power CTRL register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	GSE_LOG("MPU6050 ACC enable xyz gyro in FIFO OK!\n");

	//disable AUX_VDDIO
	databuf[0] = MPU6050C_REG_AUX_VDD;	 
	databuf[1] = MPU6050C_AUX_VDDIO_DIS;
	res = i2c_master_send(client, databuf, 0x2);
	if (res <= 0)
	{
		GSE_ERR("write AUX_VDD register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	GSE_LOG("MPU6050 ACC disable AUX_VDDIO OK!\n");

	//enable FIFO and reset FIFO
	databuf[0] = MPU6050C_ACC_REG_FIFO_CTL;	  
	databuf[1] = (MPU6050C_ACC_FIFO_EN | MPU6050C_ACC_FIFO_RST);
	res = i2c_master_send(client, databuf, 0x2);
	if (res <= 0)
	{
		GSE_ERR("write FIFO CTRL register err!\n");
		return MPU6050C_ACC_ERR_I2C;
	}

	GSE_LOG("MPU6050_ACC_FIFOConfig OK!\n");
	return MPU6050C_ACC_SUCCESS;
}

/*----------------------------------------------------------------------------*/
static int MPU6050_ACC_ReadFifoData(struct i2c_client *client, s16 *data, int* datalen)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);		  
	u8 buf[MPU6050C_ACC_DATA_LEN] = {0};
	s16 tmp1[MPU6050C_ACC_AXES_NUM] = {0}; 
	s16 tmp2[MPU6050C_ACC_AXES_NUM] = {0}; 
	int err = 0;
	u8 tmp = 0;
	int packet_cnt = 0;
	int i;
	GSE_FUN();

	if (NULL == client)
	{
		return -EINVAL;
	}

	//stop putting data in FIFO
	MPU6050_ACC_ReadStart(client, FALSE);

	//read data number of bytes in FIFO
	err = hwmsen_read_byte(client, MPU6050C_ACC_REG_FIFO_CNTH, &tmp);
	if (err)
	{
		GSE_ERR("read data high number of bytes error: %d\n", err);
		return -1;
	}
	packet_cnt = tmp<< 8;

	err = hwmsen_read_byte(client, MPU6050C_ACC_REG_FIFO_CNTL, &tmp);
	if (err)
	{
		GSE_ERR("read data low number of bytes error: %d\n", err);
		return -1;
	}
	packet_cnt = (packet_cnt + tmp) /MPU6050C_ACC_DATA_LEN;

	GSE_LOG("MPU6050 ACC Read Data packet number OK: %d\n", packet_cnt);

	*datalen = packet_cnt;

	//Within +-5% range: timing_tolerance * packet_thresh=0.05*75
	if (packet_cnt && ((abs(packet_thresh - packet_cnt)) < 50))
	{
		//read data in FIFO
		for (i = 0; i < packet_cnt; i++)
		{
			if (hwmsen_read_block(client, MPU6050C_ACC_REG_FIFO_DATA, buf, MPU6050C_ACC_DATA_LEN))
			{
				GSE_ERR("MPU6050 ACC read data from FIFO error: %d\n", err);
				return -2;
			}
			else
			{
				GSE_LOG("MPU6050 ACC read Data of diff address from FIFO OK !\n");
			}

			tmp1[MPU6050C_ACC_AXIS_X] = (s16)((buf[MPU6050C_ACC_AXIS_X*2+1]) | (buf[MPU6050C_ACC_AXIS_X*2] << 8));
			tmp1[MPU6050C_ACC_AXIS_Y] = (s16)((buf[MPU6050C_ACC_AXIS_Y*2+1]) | (buf[MPU6050C_ACC_AXIS_Y*2] << 8));
			tmp1[MPU6050C_ACC_AXIS_Z] = (s16)((buf[MPU6050C_ACC_AXIS_Z*2+1]) | (buf[MPU6050C_ACC_AXIS_Z*2] << 8));

			//remap coordinate//
			tmp2[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*tmp1[MPU6050C_ACC_AXIS_X];
			tmp2[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*tmp1[MPU6050C_ACC_AXIS_Y];
			tmp2[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*tmp1[MPU6050C_ACC_AXIS_Z];

			data[3* i +MPU6050C_ACC_AXIS_X] = tmp2[MPU6050C_ACC_AXIS_X];
			data[3* i +MPU6050C_ACC_AXIS_Y] = tmp2[MPU6050C_ACC_AXIS_Y];
			data[3* i +MPU6050C_ACC_AXIS_Z] = tmp2[MPU6050C_ACC_AXIS_Z];

			GSE_LOG("acc FIFO packet[%d]:[%04X %04X %04X] => [%5d %5d %5d]\n", i, 
					 data[3*i +MPU6050C_ACC_AXIS_X], data[3*i +MPU6050C_ACC_AXIS_Y], data[3*i +MPU6050C_ACC_AXIS_Z], 
					 data[3*i +MPU6050C_ACC_AXIS_X], data[3*i +MPU6050C_ACC_AXIS_Y], data[3*i +MPU6050C_ACC_AXIS_Z]);
		}

	}
	else
	{
		GSE_ERR("MPU6050 acc Incorrect packet count: %d\n", packet_cnt);
		return -3;
	}

	return 0;
}

//for factory mode
static int MPU6050_ACC_PROCESS_SMT_DATA(struct i2c_client *client, short *data)
{
    int total_num = 0;
    int retval =0;
    long xSum = 0;
    long ySum = 0;
    long zSum = 0;
    long xAvg, yAvg, zAvg;
    long xRMS, yRMS, zRMS;
    int i=0;

    int bias_thresh = 10484;  //5242; // 40 dps * 131.072 LSB/dps
    //float RMS_thresh = 687.19f; // (.2 dps * 131.072) ^ 2
    long RMS_thresh = 68719; // (.2 dps * 131.072) ^ 2

    total_num = data[0];
    retval = data[1];
    GSE_LOG("MPU6050 read acc data OK, total number: %d \n", total_num); 
    for (i = 0; i < total_num; i++)
    {
        xSum =xSum + data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_X +2]; 
        ySum =ySum + data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Y +2]; 
        zSum =zSum + data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Z +2]; 

        /*
        FLPLOGD("read gyro data OK: packet_num:%d, [X:%5d, Y:%5d, Z:%5d]\n", i, data[MPU6050_AXES_NUM*i + MPU6050_AXIS_X +2], 
            data[MPU6050_AXES_NUM*i + MPU6050_AXIS_Y +2], data[MPU6050_AXES_NUM*i + MPU6050_AXIS_Z +2]);
        FLPLOGD("MPU6050 xSum: %5d,  ySum: %5d, zSum: %5d \n", xSum, ySum, zSum); 
        */
    }
    GSE_LOG("MPU6050 xSum: %5ld,  ySum: %5ld, zSum: %5ld \n", xSum, ySum, zSum); 

    if (total_num != 0)
    {
        xAvg = (xSum / total_num); 
        yAvg = (ySum / total_num); 
        zAvg = (zSum / total_num);
    }
    else
    {
        xAvg = xSum;
        yAvg = ySum;
        zAvg = zSum;
    }

    GSE_LOG("MPU6050 acc xAvg: %ld,  yAvg: %ld,  zAvg: %ld \n", xAvg, yAvg, zAvg); 

    if ( abs(xAvg) > bias_thresh)
    {
        GSE_LOG("X-Gyro bias exceeded threshold \n");
        retval |= 1 << 3;
    }
    if ( abs(yAvg) >  bias_thresh)
    {
        GSE_LOG("Y-Gyro bias exceeded threshold \n");
        retval |= 1 << 4;
    }
    if ( abs(zAvg ) > bias_thresh)
    {
        GSE_LOG("Z-Gyro bias exceeded threshold \n");
        retval |= 1 << 5;
    }

    xRMS = 0; 
    yRMS = 0; 
    zRMS = 0;

    //Finally, check RMS
    for ( i = 0; i < total_num ; i++)
    {
        xRMS += (data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_X+2]-xAvg)*(data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_X+2]-xAvg);
        yRMS += (data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Y+2]-yAvg)*(data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Y+2]-yAvg);
        zRMS += (data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Z+2]-zAvg)*(data[MPU6050C_ACC_AXES_NUM*i + MPU6050C_ACC_AXIS_Z+2]-zAvg);        
    }

    GSE_LOG("MPU6050 ACC xRMS: %ld,  yRMS: %ld,  zRMS: %ld \n", xRMS, yRMS, zRMS); 
    xRMS = 100*xRMS;
    yRMS = 100*yRMS;
    zRMS = 100*zRMS;

	if (FACTORY_BOOT == get_boot_mode())
  		return retval;
    if ( xRMS > RMS_thresh * total_num)
    {
        GSE_LOG("X-Gyro RMS exceeded threshold, RMS_thresh: %ld \n", RMS_thresh * total_num);
        retval |= 1 << 6;
    }
    if ( yRMS > RMS_thresh * total_num )
    {
        GSE_LOG("Y-Gyro RMS exceeded threshold, RMS_thresh: %ld \n", RMS_thresh * total_num);
        retval |= 1 << 7;
    }
    if ( zRMS > RMS_thresh * total_num )
    {
        GSE_LOG("Z-Gyro RMS exceeded threshold, RMS_thresh: %ld \n", RMS_thresh * total_num);
        retval |= 1 << 8;
    }
    if ( xRMS == 0 || yRMS == 0 || zRMS == 0)
        //If any of the RMS noise value returns zero, then we might have dead gyro or FIFO/register failure
        retval |= 1 << 9;
    GSE_LOG("retval %ld \n", retval);
    return retval;

}



/*----------------------------------------------------------------------------*/
static int MPU6050_ACC_SMTReadSensorData(struct i2c_client *client, s16 *buf, int bufsize)
{
    //S16 gyro[MPU6050_AXES_NUM*MPU6050_FIFOSIZE];
    int res = 0;
    int i;
    int datalen, total_num= 0;
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);  

    GSE_FUN();

	if(!test_bit(CMC_BIT_GYRO, &obj->enable))
	{
		set_bit(CMC_BIT_GYRO, &obj->enable);
		MPU6050C_ACC_SetPWR_MGMT_12(client);
		msleep(50);
	}

    if (NULL == buf)
    {
        return -1;
    }
    if (NULL == client)
    {
        *buf = 0;
        return -2;
    }

    for (i = 0; i < MPU6050C_ACC_AXES_NUM; i++)
    {
        res = MPU6050_ACC_FIFOConfig(client, (i+1));
        if (res)
        {
            GSE_ERR("MPU6050_ACC_FIFOConfig error:%d!\n", res);
            return -3;
        }

        //putting data in FIFO during the delayed 600ms
        mdelay(600);

        res = MPU6050_ACC_ReadFifoData(client, &(buf[total_num+2]), &datalen);
        if (res)
        {
            if (res == (-3))
            {
                buf[1] = (1<< i);
            }
            else
            {
                GSE_ERR("MPU6050_ACC_ReadData error:%d!\n", res);
                return -3;
            }
        }
        else
        {
            buf[0] = datalen;
            total_num+=datalen*MPU6050C_ACC_AXES_NUM;
        }
    }

    GSE_LOG("gyroscope read data OK, total packet: %d", buf[0] ); 

    return 0;
}
static int mpu6050c_read_raw_data(struct i2c_client *client, s16 data[MPU6050C_ACC_AXES_NUM])
{
	char databuf[6];	
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);  

	if(!test_bit(CMC_BIT_ACC, &obj->enable))
	{
		set_bit(CMC_BIT_ACC, &obj->enable);
		MPU6050C_ACC_SetPWR_MGMT_12(client);
		msleep(50);
	}

	if(hwmsen_read_block(client, MPU6050C_ACC_REG_ACCEL_XOUT_H, databuf, 6))
	{
		GSE_ERR("MPU6050C_ACC read accscope data	error\n");
		return -2;
	}
	else
	{
		data[MPU6050C_ACC_AXIS_X] = ((s16)((databuf[MPU6050C_ACC_AXIS_X*2+1]) | (databuf[MPU6050C_ACC_AXIS_X*2] << 8)));
		data[MPU6050C_ACC_AXIS_Y] = ((s16)((databuf[MPU6050C_ACC_AXIS_Y*2+1]) | (databuf[MPU6050C_ACC_AXIS_Y*2] << 8)));
		data[MPU6050C_ACC_AXIS_Z] = ((s16)((databuf[MPU6050C_ACC_AXIS_Z*2+1]) | (databuf[MPU6050C_ACC_AXIS_Z*2] << 8)));
	}
	return 0;

}


/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ReadAccData(struct i2c_client *client, char *buf, int bufsize)
{
	char databuf[6];	
	int data[3];
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);  

	if(!test_bit(CMC_BIT_ACC, &obj->enable))
	{
		set_bit(CMC_BIT_ACC, &obj->enable);
		MPU6050C_ACC_SetPWR_MGMT_12(client);
		msleep(50);
	}

	if(hwmsen_read_block(client, MPU6050C_ACC_REG_ACCEL_XOUT_H, databuf, 6))
	{
		GSE_ERR("MPU6050C_ACC read accscope data	error\n");
		return -2;
	}
	else
	{
		obj->data[MPU6050C_ACC_AXIS_X] = ((s16)((databuf[MPU6050C_ACC_AXIS_X*2+1]) | (databuf[MPU6050C_ACC_AXIS_X*2] << 8)));
		obj->data[MPU6050C_ACC_AXIS_Y] = ((s16)((databuf[MPU6050C_ACC_AXIS_Y*2+1]) | (databuf[MPU6050C_ACC_AXIS_Y*2] << 8)));
		obj->data[MPU6050C_ACC_AXIS_Z] = ((s16)((databuf[MPU6050C_ACC_AXIS_Z*2+1]) | (databuf[MPU6050C_ACC_AXIS_Z*2] << 8)));
#if 1		
		if(atomic_read(&obj->trace) & ACC_TRC_RAWDATA)
		{
			GSE_LOG("read acc register: %d, %d, %d, %d, %d, %d",
				databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5]);
			GSE_LOG("get acc raw data (0x%08X, 0x%08X, 0x%08X) -> (%5d, %5d, %5d)\n", 
				obj->data[MPU6050C_ACC_AXIS_X],obj->data[MPU6050C_ACC_AXIS_Y],obj->data[MPU6050C_ACC_AXIS_Z],
				obj->data[MPU6050C_ACC_AXIS_X],obj->data[MPU6050C_ACC_AXIS_Y],obj->data[MPU6050C_ACC_AXIS_Z]);
			GSE_LOG("acc_cali_sw[0] =%d,acc_cali_sw[1] =%d,acc_cali_sw[2] =%d,\n", 
				obj->acc_cali_sw[0], obj->acc_cali_sw[1], obj->acc_cali_sw[2]);
		}
#endif		
		/*remap coordinate*/
		data[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->data[MPU6050C_ACC_AXIS_X];
		data[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->data[MPU6050C_ACC_AXIS_Y];
		data[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->data[MPU6050C_ACC_AXIS_Z];

		////Out put the mg
		data[MPU6050C_ACC_AXIS_X] = data[MPU6050C_ACC_AXIS_X] * GRAVITY_EARTH_1000 / 16384;
		data[MPU6050C_ACC_AXIS_Y] = data[MPU6050C_ACC_AXIS_Y] * GRAVITY_EARTH_1000 / 16384;
		data[MPU6050C_ACC_AXIS_Z] = data[MPU6050C_ACC_AXIS_Z] * GRAVITY_EARTH_1000 / 16384;

		/* compensate data */
		if(gs_cali.valid == 1){
			data[MPU6050C_ACC_AXIS_X] -= gs_cali.x;
			data[MPU6050C_ACC_AXIS_Y] -= gs_cali.y;
			data[MPU6050C_ACC_AXIS_Z] -= gs_cali.z;
		}
	}
	
	sprintf(buf, "%04x %04x %04x", data[MPU6050C_ACC_AXIS_X],data[MPU6050C_ACC_AXIS_Y],data[MPU6050C_ACC_AXIS_Z]);

#if DEBUG		
	if(atomic_read(&obj->trace) & ACC_TRC_DATA)
	{
		GSE_LOG("get acc data packet:[%d %d %d]\n", data[0], data[1], data[2]);
	}
#endif
	
	return 0;
	
}

/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ReadGyroData(struct i2c_client *client, char *buf, int bufsize)
{
	char databuf[6];	
	int data[3];
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);  
	
	if(!test_bit(CMC_BIT_GYRO, &obj->enable))
	{
		set_bit(CMC_BIT_GYRO, &obj->enable);
		MPU6050C_ACC_SetPWR_MGMT_12(client);
		msleep(50);
	}

	if(hwmsen_read_block(client, MPU6050C_ACC_REG_GYRO_XOUT_H, databuf, 6))
	{
		GSE_ERR("MPU6050C_ACC read gyroscope data  error\n");
		return -2;
	}
	else
	{
		obj->data[MPU6050C_ACC_AXIS_X] = ((s16)((databuf[MPU6050C_ACC_AXIS_X*2+1]) | (databuf[MPU6050C_ACC_AXIS_X*2] << 8)));
		obj->data[MPU6050C_ACC_AXIS_Y] = ((s16)((databuf[MPU6050C_ACC_AXIS_Y*2+1]) | (databuf[MPU6050C_ACC_AXIS_Y*2] << 8)));
		obj->data[MPU6050C_ACC_AXIS_Z] = ((s16)((databuf[MPU6050C_ACC_AXIS_Z*2+1]) | (databuf[MPU6050C_ACC_AXIS_Z*2] << 8)));
#if DEBUG		
		if(atomic_read(&obj->trace) & ACC_TRC_RAWDATA)
		{
			GSE_LOG("read gyro register: %d, %d, %d, %d, %d, %d",
				databuf[0], databuf[1], databuf[2], databuf[3], databuf[4], databuf[5]);
			GSE_LOG("get gyro raw data (0x%08X, 0x%08X, 0x%08X) -> (%5d, %5d, %5d)\n", 
				obj->data[MPU6050C_ACC_AXIS_X],obj->data[MPU6050C_ACC_AXIS_Y],obj->data[MPU6050C_ACC_AXIS_Z],
				obj->data[MPU6050C_ACC_AXIS_X],obj->data[MPU6050C_ACC_AXIS_Y],obj->data[MPU6050C_ACC_AXIS_Z]);
			GSE_LOG("gyro_cali_sw[0] =%d,gyro_cali_sw[1] =%d,gyro_cali_sw[2] =%d,\n",
				obj->gyro_cali_sw[0], obj->gyro_cali_sw[1], obj->gyro_cali_sw[2]);

		}
#endif		
		obj->data[MPU6050C_ACC_AXIS_X] = obj->data[MPU6050C_ACC_AXIS_X] + obj->gyro_cali_sw[MPU6050C_ACC_AXIS_X];
		obj->data[MPU6050C_ACC_AXIS_Y] = obj->data[MPU6050C_ACC_AXIS_Y] + obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Y];
		obj->data[MPU6050C_ACC_AXIS_Z] = obj->data[MPU6050C_ACC_AXIS_Z] + obj->gyro_cali_sw[MPU6050C_ACC_AXIS_Z];

		/*remap coordinate*/
		data[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]*obj->data[MPU6050C_ACC_AXIS_X];
		data[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]*obj->data[MPU6050C_ACC_AXIS_Y];
		data[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]*obj->data[MPU6050C_ACC_AXIS_Z];

		//Out put the degree/second(o/s)
		data[MPU6050C_ACC_AXIS_X] = data[MPU6050C_ACC_AXIS_X] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;
		data[MPU6050C_ACC_AXIS_Y] = data[MPU6050C_ACC_AXIS_Y] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;
		data[MPU6050C_ACC_AXIS_Z] = data[MPU6050C_ACC_AXIS_Z] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;

	
	}

	sprintf(buf, "%04x %04x %04x", data[MPU6050C_ACC_AXIS_X],data[MPU6050C_ACC_AXIS_Y],data[MPU6050C_ACC_AXIS_Z]);

#if DEBUG		
	if(atomic_read(&obj->trace) & ACC_TRC_DATA)
	{
		GSE_LOG("get gyro data packet:[%d %d %d]\n", data[0], data[1], data[2]);
	}
#endif
	
	return 0;
	
}

//for factory mode
static int mpu6050c_set_powermode(struct i2c_client *client,int value)
{
	int err=0;
	
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);  

	if(value == true)
	{
		set_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable);
	}
	else
	{
		clear_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable);
	}
		
	err = MPU6050C_ACC_SetPWR_MGMT_12(client);
	return err;
}
/*----------------------------------------------------------------------------*/
static int MPU6050C_ACC_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "MPU6050C_ACC Chip");
	return 0;
}


/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mpu6050c_acc_i2c_client;
	char strbuf[MPU6050C_ACC_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	MPU6050C_ACC_ReadChipInfo(client, strbuf, MPU6050C_ACC_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = mpu6050c_acc_i2c_client;
	char strbuf[MPU6050C_ACC_BUFSIZE];
	
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	
	MPU6050C_ACC_ReadAccData(client, strbuf, MPU6050C_ACC_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);;            
}

/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct mpu6050c_acc_i2c_data *obj = mpu6050c_obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct mpu6050c_acc_i2c_data *obj = mpu6050c_obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct mpu6050c_acc_i2c_data *obj = mpu6050c_obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);   
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_reg_value(struct device_driver *ddri, char *buf)
{
	int i = 0,count = 0;
	u8 bufdata;
	struct mpu6050c_acc_i2c_data *obj = mpu6050c_obj_i2c_data;
	
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}	

	for(i = 0;i < 118; i++)
	{
		hwmsen_read_block(obj->client, 0x00+i, &bufdata, 1);
		count+= sprintf(buf+count,"[%x] = (%x)\n",0x00+i, bufdata);
	}

	return count;
}
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,             S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata,           S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(reg,	 	 S_IWUSR | S_IRUGO, show_reg_value, NULL);
/*----------------------------------------------------------------------------*/
static struct driver_attribute *MPU6050C_ACC_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/	
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_reg,
};
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(MPU6050C_ACC_attr_list)/sizeof(MPU6050C_ACC_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(0 != (err = driver_create_file(driver, MPU6050C_ACC_attr_list[idx])))
		{            
			GSE_ERR("driver_create_file (%s) = %d\n", MPU6050C_ACC_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(MPU6050C_ACC_attr_list)/sizeof(MPU6050C_ACC_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, MPU6050C_ACC_attr_list[idx]);
	}
	

	return err;
}

/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_gpio_config(void)
{
    //because we donot use EINT ,to support low power
    // config to GPIO input mode + PD    
    //set   GPIO_MSE_EINT_PIN
#if 0
    mt_set_gpio_mode(GPIO_GYRO_EINT_PIN, GPIO_GYRO_EINT_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_GYRO_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_GYRO_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_GYRO_EINT_PIN, GPIO_PULL_DOWN);
#endif
	return 0;
}
static int mpu6050c_acc_init_client(struct i2c_client *client, bool enable)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;
	GSE_FUN();	
	
	res = MPU6050C_ACC_SetPWR_MGMT_12(client);
	if(res != MPU6050C_ACC_SUCCESS)
	{
		return res;
	}
	
	res = MPU6050C_ACC_SetAccDataRange(client,MPU6050C_ACC_ACC_RNG_2G);
	if(res != MPU6050C_ACC_SUCCESS) 
	{
		GSE_ERR("MPU6050C_ACC_SetAccDataRange failed!\n");
		return res;
	}

	if(obj->mpu_chipid == MPU6880_CHIP_ID){	//MPU6880
		res = MPU6880_SetDataBandWidth_Accel(client,0x0A);
		if(res != MPU6050C_ACC_SUCCESS) 
		{
			GSE_ERR("MPU6880C_ACC_SetDataBandWidth failed!\n");
			return res;
		}
	}
	
	res = MPU6050C_ACC_SetGyroDataRange(client,MPU6050C_ACC_GYRO_RNG_1000);
	if(res != MPU6050C_ACC_SUCCESS) 
	{
		GSE_ERR("MPU6050C_ACC_SetGyroDataRange failed!\n");
		return res;
	}
	res = MPU6050C_ACC_SetDataBandWidth(client,MPU6050C_ACC_RATE_1K_LPFB_98HZ);
	if(res != MPU6050C_ACC_SUCCESS) 
	{
		GSE_ERR("MPU6050C_ACC_SetDataBandWidth failed!\n");
		return res;
	}

	// Set 125HZ sample rate
	res = MPU6050C_ACC_SetSampleRate(client, 125);
	if(res != MPU6050C_ACC_SUCCESS ) 
	{
		GSE_ERR("MPU6050C_ACC_SetSampleRate failed!\n");
		return res;
	}
	GSE_LOG("mpu6050c_acc_init_client OK!\n");

#ifdef CONFIG_MPU6050C_ACC_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif

	return MPU6050C_ACC_SUCCESS;
}

static int mpu6050c_remap_to_gravity(struct i2c_client *client, int data[MPU6050C_ACC_AXES_NUM])
{
	struct mpu6050c_acc_i2c_data *obj = (struct mpu6050c_acc_i2c_data*)i2c_get_clientdata(client);	
	int acc_tmp[MPU6050C_ACC_AXES_NUM]={0};

	if(NULL == client)
	{
		return -EINVAL;
	}
	
	/*remap coordinate*/
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]* data[MPU6050C_ACC_AXIS_X];
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]* data[MPU6050C_ACC_AXIS_Y];
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]* data[MPU6050C_ACC_AXIS_Z];

	//Out put the mg
	data[MPU6050C_ACC_AXIS_X] = acc_tmp[MPU6050C_ACC_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
	data[MPU6050C_ACC_AXIS_Y] = acc_tmp[MPU6050C_ACC_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
	data[MPU6050C_ACC_AXIS_Z] = acc_tmp[MPU6050C_ACC_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;

	return 0;
}


static int mpu6050c_get_cali_offset(struct i2c_client *client,SENSOR_DATA *sensor_data)
{
	struct mpu6050c_acc_i2c_data *obj = (struct mpu6050c_acc_i2c_data*)i2c_get_clientdata(client);	

	int count = 20;
	s16 data[MPU6050C_ACC_AXES_NUM] = {0,0,0};
	int err = 0;
	int acc_sum[MPU6050C_ACC_AXES_NUM]={0};
	int acc_avg[MPU6050C_ACC_AXES_NUM]={0};
	int acc_raw[20][MPU6050C_ACC_AXES_NUM]={0};
	int i=0, j=0, offset=0;
	int cali_tolerance = 20; // 20%

	for(i=0; i<count; i++)
	{
		err= mpu6050c_read_raw_data(client,data);
		if (err) {
			err = -EFAULT;
			return err;
		}
		acc_sum[0] += (int)data[0];
		acc_sum[1] += (int)data[1];
		acc_sum[2] += (int)data[2];
	
		acc_raw[i][MPU6050C_ACC_AXIS_X] = (int)data[0];
		acc_raw[i][MPU6050C_ACC_AXIS_Y] = (int)data[1];
		acc_raw[i][MPU6050C_ACC_AXIS_Z] = (int)data[2];
		
		msleep(50);
	}
		acc_avg[MPU6050C_ACC_AXIS_X] = acc_sum[0] / count;
		acc_avg[MPU6050C_ACC_AXIS_Y] = acc_sum[1] / count;
		acc_avg[MPU6050C_ACC_AXIS_Z] = acc_sum[2] / count;
	
		err = mpu6050c_remap_to_gravity(client, acc_avg);
		if(err){
			err = -EFAULT;
			return err;
		}
	
	for(i=0; i<count; i++)
	{
		err = mpu6050c_remap_to_gravity(client, &acc_raw[i][0]);
		if(err){
			err = -EFAULT;
			return err;
		}
	}
	
	for(j=0;j<MPU6050C_ACC_AXES_NUM;j++)
	{
		for(i=0;i<count;i++)
		{
			offset = abs(acc_raw[i][j]- acc_avg[j]);
			if(offset > (GRAVITY_EARTH_1000 * cali_tolerance / 100)){
				return -EFAULT;
			}
		}
	}

	sensor_data->x = acc_avg[MPU6050C_ACC_AXIS_X];
	sensor_data->y = acc_avg[MPU6050C_ACC_AXIS_Y];
	sensor_data->z = acc_avg[MPU6050C_ACC_AXIS_Z] - GRAVITY_EARTH_1000;

	return 0;
}

/*----------------------------------------------------------------------------*/
int mpu6050c_acc_acc_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;
	struct mpu6050c_acc_i2c_data *priv = (struct mpu6050c_acc_i2c_data*)self;
	hwm_sensor_data* acc_data;
	char buff[MPU6050C_ACC_BUFSIZE];	

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
			
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable accscope parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				//GSE_LOG("mpu6050c_acc_acc_operate SENSOR_ENABLE =%d",value);
				if(((value == 0) && (!test_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable))) 
					||((value == 1) && (test_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable))))
				{
					GSE_LOG("accscope device have updated!\n");
				}
				else
				{
					if(value == true)
					{
						set_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable);
					}
					else
					{
						clear_bit(CMC_BIT_ACC, &mpu6050c_obj_i2c_data->enable);
					}
					err = MPU6050C_ACC_SetPWR_MGMT_12(priv->client);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get accscope data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				acc_data = (hwm_sensor_data *)buff_out;
				MPU6050C_ACC_ReadAccData(priv->client, buff, MPU6050C_ACC_BUFSIZE);
				sscanf(buff, "%x %x %x", &acc_data->values[0], 
					&acc_data->values[1], &acc_data->values[2]);				
				acc_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				acc_data->value_divide = 1000;
				//GSE_ERR("X :%d,Y: %d, Z: %d\n",gsensor_data->values[0],gsensor_data->values[1],gsensor_data->values[2]);
			}
			break;
		default:
			GSE_ERR("accscope operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

int mpu6050c_acc_gyro_operate(void* self, uint32_t command, void* buff_in, int size_in,
		void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value;	
	struct mpu6050c_acc_i2c_data *priv = (struct mpu6050c_acc_i2c_data*)self;
	hwm_sensor_data* gyro_data;
	char buff[MPU6050C_ACC_BUFSIZE];	

	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
			
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable gyroscope parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				//GSE_LOG("mpu6050c_acc_gyro_operate SENSOR_ENABLE =%d",value);
				if(((value == 0) && (!test_bit(CMC_BIT_GYRO, &mpu6050c_obj_i2c_data->enable))) 
					||((value == 1) && (test_bit(CMC_BIT_GYRO, &mpu6050c_obj_i2c_data->enable))))
				{
					GSE_LOG("accscope device have updated!\n");
				}
				else
				{
					if(value == true)
					{
						set_bit(CMC_BIT_GYRO, &mpu6050c_obj_i2c_data->enable);
					}
					else
					{
						clear_bit(CMC_BIT_GYRO, &mpu6050c_obj_i2c_data->enable);
					}
					err = MPU6050C_ACC_SetPWR_MGMT_12(priv->client);
				}
			}
			break;


		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get gyroscope data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gyro_data = (hwm_sensor_data *)buff_out;
				MPU6050C_ACC_ReadGyroData(priv->client, buff, MPU6050C_ACC_BUFSIZE);
				sscanf(buff, "%x %x %x", &gyro_data->values[0], 
									&gyro_data->values[1], &gyro_data->values[2]);				
				gyro_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;				
				gyro_data->value_divide = DEGREE_TO_RAD;
			}
			break;
		default:
			GSE_ERR("gyroscope operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

/****************************************************************************** 
 * Function Configuration
******************************************************************************/
static int mpu6050c_acc_open(struct inode *inode, struct file *file)
{
	file->private_data = mpu6050c_acc_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_remap_to_gravity(struct i2c_client *client, int data[MPU6050C_ACC_AXES_NUM])
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);
	int acc_tmp[MPU6050C_ACC_AXES_NUM]={0};

	if(NULL == client)
	{
		return -EINVAL;
	}

	/*remap coordinate*/
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_X]] = obj->cvt.sign[MPU6050C_ACC_AXIS_X]* data[MPU6050C_ACC_AXIS_X];
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_Y]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Y]* data[MPU6050C_ACC_AXIS_Y];
	acc_tmp[obj->cvt.map[MPU6050C_ACC_AXIS_Z]] = obj->cvt.sign[MPU6050C_ACC_AXIS_Z]* data[MPU6050C_ACC_AXIS_Z];

	//Out put the mg
	data[MPU6050C_ACC_AXIS_X] = acc_tmp[MPU6050C_ACC_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
	data[MPU6050C_ACC_AXIS_Y] = acc_tmp[MPU6050C_ACC_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
	data[MPU6050C_ACC_AXIS_Z] = acc_tmp[MPU6050C_ACC_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;

	return 0;
}
/*----------------------------------------------------------------------------*/
//static int mpu6050c_acc_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long mpu6050c_acc_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct mpu6050c_acc_i2c_data *obj = (struct mpu6050c_acc_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[MPU6050C_ACC_BUFSIZE] = {0};
	s16 *SMTdata;
	void __user *data;
	long err = 0;
	int copy_cnt = 0;
	SENSOR_DATA sensor_data;
	int cali[3];
	int smtRes=0;
	s16 raw_data[MPU6050C_ACC_AXES_NUM];
	int acc[MPU6050C_ACC_AXES_NUM] = {0};
	uint32_t enable;
	struct GS_CALI_DATA_STRUCT gs_cali_temp;
	int acc_tmp[MPU6050C_ACC_AXES_NUM] = {0};
	//GSE_FUN();
	
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}


	switch(cmd)
	{
		case GYROSCOPE_IOCTL_INIT:
			mpu6050c_acc_init_client(client, false);			
			break;
		case GYROSCOPE_IOCTL_SMT_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			SMTdata = kzalloc(sizeof(*SMTdata) * 800, GFP_KERNEL);
			if (SMTdata == NULL)
			{
				err = -ENOMEM;
				break;
			}
			memset(SMTdata, 0, sizeof(*SMTdata) * 800);
			mdelay(50); //add by MTK
			MPU6050_ACC_SMTReadSensorData(client, SMTdata, 800);
			//GSE_LOG("gyroscope read data from kernel OK: sizeof:%d, strlen:%d, packet:%d!\n", 
			//sizeof(SMTdata), strlen(SMTdata), SMTdata[0]);
			GSE_LOG("gyroscope read data from kernel OK: SMTdata[0]:%d, copied packet:%d!\n", SMTdata[0],
					 ((SMTdata[0]*MPU6050C_ACC_AXES_NUM+2)*sizeof(s16)+1));
			
			smtRes = MPU6050_ACC_PROCESS_SMT_DATA(client,SMTdata);
			GSE_LOG("ioctl smtRes: %d!\n", smtRes);			
			copy_cnt = copy_to_user(data, &smtRes,  sizeof(smtRes));
			kfree(SMTdata);
			if(copy_cnt)
			{
				err = -EFAULT;
				GSE_ERR("copy gyro data to user failed!\n");
			}	
			GSE_LOG("copy gyro data to user OK: %d!\n", copy_cnt);

			break;
		case GYROSCOPE_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			MPU6050C_ACC_ReadGyroData(client, strbuf, MPU6050C_ACC_BUFSIZE);
			if(copy_to_user(data, strbuf, sizeof(strbuf)))
			{
				err = -EFAULT;
				break;	  
			}				 
			break;

		case GYROSCOPE_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			
			else
			{
				GSE_LOG("fwq going to set cali\n");
				cali[MPU6050C_ACC_AXIS_X] = sensor_data.x * MPU6050C_DEFAULT_LSB / MPU6050C_FS_MAX_LSB;
				cali[MPU6050C_ACC_AXIS_Y] = sensor_data.y * MPU6050C_DEFAULT_LSB / MPU6050C_FS_MAX_LSB;
				cali[MPU6050C_ACC_AXIS_Z] = sensor_data.z * MPU6050C_DEFAULT_LSB / MPU6050C_FS_MAX_LSB;			  
				err = MPU6050C_ACC_WriteGyroCalibration(client, cali);			 
				GSE_LOG("fwq GSENSOR_IOCTL_SET_CALI!!sensor_data .x =%d,sensor_data .z =%d,sensor_data .z =%d \n",sensor_data.x,sensor_data.y,sensor_data.z);
			}
			break;

		case GYROSCOPE_IOCTL_CLR_CALI:
			err = MPU6050C_ACC_ResetGyroCalibration(client);
			break;

		case GYROSCOPE_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			err = MPU6050C_ACC_ReadGyroCalibration(client, cali);
			if(err)
			{
				break;
			}
			
			sensor_data.x = cali[MPU6050C_ACC_AXIS_X] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;
			sensor_data.y = cali[MPU6050C_ACC_AXIS_Y] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;
			sensor_data.z = cali[MPU6050C_ACC_AXIS_Z] * MPU6050C_FS_MAX_LSB / MPU6050C_DEFAULT_LSB;
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}		
			break;
			case GSENSOR_IOCTL_INIT:
				mpu6050c_acc_init_client(client, false);
			break;
		
			case GSENSOR_SET_ACC_MODE://whf add start
				data = (void __user *) arg;
				if(copy_from_user(&enable, data, sizeof(enable)))
				{
					err = -EFAULT;
					break;
				}
				err=mpu6050c_set_powermode(client,enable);
				if (err)
					GSE_ERR("set power mode failed, err = %d\n", err);
				
				break;

			case GSENSOR_IOCTL_READ_CHIPINFO:
				GSE_LOG("fwq GSENSOR_IOCTL_READ_CHIPINFO\n");
				data = (void __user *) arg;
				if(data == NULL)
				{
					err = -EINVAL;
					break;	  
				}
				
				MPU6050C_ACC_ReadChipInfo(client, strbuf, MPU6050C_ACC_BUFSIZE);
				if(copy_to_user(data, strbuf, strlen(strbuf)+1))
				{
					err = -EFAULT;
					break;
				}				 

			break;
			case GSENSOR_IOCTL_READ_SENSORDATA:
				data = (void __user *) arg;
				if(data == NULL)
				{
					err = -EINVAL;
					break;	  
				}
				MPU6050C_ACC_ReadAccData(client, strbuf, MPU6050C_ACC_BUFSIZE);
				if(copy_to_user(data, strbuf, strlen(strbuf)+1))
				{
					err = -EFAULT;
					break;	  
				}				 
			break;
			case GSENSOR_IOCTL_READ_OFFSET:
				
			break;
			case GSENSOR_IOCTL_READ_RAW_DATA:
				data = (void __user *) arg;
				if (data == NULL) {
					err = -EINVAL;
					break;
				}

				err = mpu6050c_read_raw_data(client, raw_data);
				if (err) {
					err = -EFAULT;
					break;
				}
				acc_tmp[0] = (int)raw_data[0];
				acc_tmp[1] = (int)raw_data[1];
				acc_tmp[2] = (int)raw_data[2];
				
				err = mpu6050c_acc_remap_to_gravity(client, acc_tmp);
				if(err){
					err = -EFAULT;
					break;
				}
				sensor_data.x = acc_tmp[0];
				sensor_data.y = acc_tmp[1];
				sensor_data.z = acc_tmp[2];

				if (copy_to_user(data, &sensor_data, sizeof(sensor_data))) {
					err = -EFAULT;
					break;
				}
				
			break;
		case GSENSOR_IOCTL_SET_CALI_NEW:
			GSE_LOG("GSENSOR_IOCTL_SET_CALI_NEW!!\n");
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&gs_cali_temp, data, sizeof(gs_cali_temp)))
			{
				err = -EFAULT;
				break;	  
			}
				/* get remapped value in NV (mg) */
				err = MPU6050C_ACC_WriteAccCalibration_New(client, &gs_cali_temp);
				if(err){
					err = -EFAULT;
					break;
				}
			GSE_LOG("SET_CALI_NEW OK!valid=%d => x=%d,y=%d,z=%d\n",gs_cali.valid,gs_cali.x,gs_cali.y,gs_cali.z);
			break;		
			#if 0
			case GSENSOR_IOCTL_CLR_CALI:
				GSE_LOG("fwq GSENSOR_IOCTL_CLR_CALI!!\n");
				err = MPU6050C_ACC_ResetCalibration(client);
			break;
			#endif
			#if 0
			case GSENSOR_IOCTL_GET_CALI:
				GSE_LOG("fwq GSENSOR_IOCTL_GET_CALI\n");
				data = (void __user*)arg;
				if(data == NULL)
				{
					err = -EINVAL;
					break;	  
				}
				if(err = MPU6050C_ACC_ReadCalibration(client, cali))
				{
					break;
				}
				
				sensor_data.x = cali[MPU6050C_ACC_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
				sensor_data.y = cali[MPU6050C_ACC_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
				sensor_data.z = cali[MPU6050C_ACC_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
				if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
				{
					err = -EFAULT;
					break;
				}		

			break;
			#endif
		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;			
	}
	return err;
}


/*----------------------------------------------------------------------------*/
static struct file_operations mpu6050c_acc_fops = {
//	.owner = THIS_MODULE,//modified
	.open = mpu6050c_acc_open,
	.release = mpu6050c_acc_release,
	.unlocked_ioctl = mpu6050c_acc_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice mpu6050c_acc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &mpu6050c_acc_fops,
};
#if 1
/*----------------------------------------------------------------------------*/

static struct miscdevice mpu6050c_acc_gyro_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gyroscope",
	.fops = &mpu6050c_acc_fops,
};
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_suspend(struct i2c_client *client, pm_message_t msg) 
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);    
	int err; 
	GSE_FUN();   

	if(msg.event == PM_EVENT_SUSPEND)
	{   
		if(obj == NULL)
		{
			GSE_ERR("null pointer!!\n");
			return -EINVAL;
		}
		atomic_set(&obj->suspend, 1);		

		err = MPU6050C_ACC_SetPWR_MGMT_12(client);
		if(err <= 0)
		{
			return err;
		}
	}
	return 0;//modified
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_resume(struct i2c_client *client)
{
	struct mpu6050c_acc_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}

	MPU6050C_ACC_power(obj->hw, 1);
	err = mpu6050c_acc_init_client(client, false);
	if(err)
	{
		GSE_ERR("initialize client fail!!\n");
		return err;        
	}
	atomic_set(&obj->suspend, 0);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void mpu6050c_acc_early_suspend(struct early_suspend *h) 
{
	struct mpu6050c_acc_i2c_data *obj = container_of(h, struct mpu6050c_acc_i2c_data, early_drv);   
	int err;
	u8 databuf1[2]={0};
	u8 databuf2[2]={0};
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}
	atomic_set(&obj->suspend, 1);


	databuf1[0] |= MPU6050C_ACC_SLEEP;
	databuf1[0] |= MPU6050C_ACC_CLKSEL_PLL_X;
	

	databuf1[1] = databuf1[0];
	databuf1[0] = MPU6050C_ACC_REG_PWR_MGMT_1;  
	err = i2c_master_send(obj->client, databuf1, 0x2);
	if(err <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return MPU6050C_ACC_ERR_I2C;
	}
	
	databuf2[0] = 0x00;
	databuf2[1] = databuf2[0];
	databuf2[0] = MPU6050C_ACC_REG_PWR_MGMT_2;
	err = i2c_master_send(obj->client, databuf2, 0x2);
	if(err <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return MPU6050C_ACC_ERR_I2C;
	}

	MPU6050C_ACC_power(obj->hw, 0);
}
/*----------------------------------------------------------------------------*/
static void mpu6050c_acc_late_resume(struct early_suspend *h)
{
	struct mpu6050c_acc_i2c_data *obj = container_of(h, struct mpu6050c_acc_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return;
	}

	MPU6050C_ACC_power(obj->hw, 1);
	err = mpu6050c_acc_init_client(obj->client, false);
	if(err)
	{
		GSE_ERR("initialize client fail! err code %d!\n", err);
		return;        
	}
	atomic_set(&obj->suspend, 0);    
}
/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
/*static int mpu6050c_acc_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) 
{    
	strcpy(info->type, MPU6050C_ACC_DEV_NAME);
	return 0;
}
*/
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct mpu6050c_acc_i2c_data *obj;
	struct hwmsen_object sobj;
	struct hwmsen_object sobj_acc, sobj_gyro;

	int err = 0;
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct mpu6050c_acc_i2c_data));

	obj->hw = get_cust_acc_hw();
	err = hwmsen_get_convert(obj->hw->direction, &obj->cvt);
	if(err)
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	
	obj->enable = 0;
	mpu6050c_obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);
	
	set_bit(CMC_BIT_ACC, &obj->enable);
	set_bit(CMC_BIT_GYRO, &obj->enable);

	mpu6050c_acc_i2c_client = new_client;

	//mpu6050c_acc_gpio_config();
	err = MPU6050C_ACC_Reset(client);
	if(err){
		goto exit_init_failed;
	}
	err = MPU6050C_Get_ChipId(new_client);
	if(err){
		goto exit_init_failed;
	}
	err = mpu6050c_acc_init_client(new_client, false);
	if(err){
		goto exit_init_failed;
	}

	{
	 if(obj->mpu_chipid == MPU6050C_CHIP_ID){
		hardwareinfo_set_prop(HARDWARE_ACCELEROMETER, "MPU6050");
		hardwareinfo_set_prop(HARDWARE_GYROSCOPE, "MPU6050");
	 }
	 else if(obj->mpu_chipid == MPU6880_CHIP_ID){
		hardwareinfo_set_prop(HARDWARE_ACCELEROMETER, "MPU6880");
		hardwareinfo_set_prop(HARDWARE_GYROSCOPE, "MPU6880");
	 }
	}

	err = misc_register(&mpu6050c_acc_device);
	if(err)
	{
		GSE_ERR("mpu6050c_acc_device misc register failed!\n");
		goto exit_misc_device_register_failed;
	}
#if 1
	err = misc_register(&mpu6050c_acc_gyro_device);
	if(err)
	{
		GSE_ERR("mpu6050c_acc_gyro_device misc register failed!\n");
		goto exit_misc_device_register_failed;
	}
#endif
	err = mpu6050c_acc_create_attr(&mpu6050c_acc_driver.driver);
	if(err)
	{
		GSE_ERR("mpu6050c_acc create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}
	
	sobj_acc.self = obj;
    sobj_acc.polling = 1;
    sobj_acc.sensor_operate = mpu6050c_acc_acc_operate;
	err = hwmsen_attach(ID_ACCELEROMETER, &sobj_acc);
	if(err)
	{
		GSE_ERR("hwmsen_attach fail = %d\n", err);
		goto exit_kfree;
	}
	sobj_gyro.self = obj;
    sobj_gyro.polling = 1;
    sobj_gyro.sensor_operate = mpu6050c_acc_gyro_operate;
	err = hwmsen_attach(ID_GYROSCOPE, &sobj_gyro);
	if(err)
	{
		GSE_ERR("hwmsen_attach fail = %d\n", err);
		goto exit_kfree;
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_STOP_DRAWING - 2,
	obj->early_drv.suspend  = mpu6050c_acc_early_suspend,
	obj->early_drv.resume   = mpu6050c_acc_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&mpu6050c_acc_device);
	
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);        
	return err;
}

/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
	
	err = mpu6050c_acc_delete_attr(&mpu6050c_acc_driver.driver);
	if(err)
	{
		GSE_ERR("mpu6050c_acc_delete_attr fail: %d\n", err);
	}
	
	err = misc_deregister(&mpu6050c_acc_device);
	if(err)
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}
#if 1
	err = misc_deregister(&mpu6050c_acc_gyro_device);
	if(err)
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}
#endif
	err = hwmsen_detach(ID_ACCELEROMETER);
	if(err)
	{
		GSE_ERR("hwmsen_detach fail: %d\n", err);
	}
	
	err = hwmsen_detach(ID_GYROSCOPE);
	if(err)
	{
		GSE_ERR("hwmsen_detach fail: %d\n", err);
	}

	mpu6050c_acc_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_probe(struct platform_device *pdev) 
{
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_FUN();

	MPU6050C_ACC_power(hw, 1);


	if(MPU6050C_Gyro_GetI2CData()!= NULL)
	{
		return 0;
	}
	else
	{
		if(i2c_add_driver(&mpu6050c_acc_i2c_driver))
		{
			GSE_ERR("add driver error\n");
			return -1;
		}
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int mpu6050c_acc_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();    
    MPU6050C_ACC_power(hw, 0);
    i2c_del_driver(&mpu6050c_acc_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver mpu6050c_acc_driver = {
	.probe      = mpu6050c_acc_probe,
	.remove     = mpu6050c_acc_remove,    
	.driver     = {
		.name  = "gsensor",
		.owner = THIS_MODULE,//modified
	}
};

/*----------------------------------------------------------------------------*/
static int __init mpu6050c_acc_init(void)
{
	//GSE_FUN();
	struct acc_hw *hw = get_cust_acc_hw();
	GSE_LOG("%s: i2c_number=%d\n", __func__,hw->i2c_num); 
	i2c_register_board_info(hw->i2c_num, &i2c_mpu6050c_acc, 1);
	if(platform_driver_register(&mpu6050c_acc_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit mpu6050c_acc_exit(void)
{
	GSE_FUN();
	platform_driver_unregister(&mpu6050c_acc_driver);
}
/*----------------------------------------------------------------------------*/
module_init(mpu6050c_acc_init);
module_exit(mpu6050c_acc_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MPU6050C_ACC  driver");
MODULE_AUTHOR("Chunlei.Wang@mediatek.com");
