/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <linux/ioctl.h>	
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "libfile_op.h"
#include "CFG_GS_File.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <linux/input.h>
#include <math.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef CUSTOM_KERNEL_ACCELEROMETER
#include <linux/sensors_io.h>
#include <linux/hwmsensor.h>
#include "libhwm.h"

#define GRAVITY_EARTH_1000 		9807  // about (9.80665f)*1000

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[GSC] "
#define mod_to_gsc_data(p) (struct gsc_data*)((char*)(p) + sizeof(struct ftm_module))
#define GSCLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define GSCLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_CLEAR,
		ITEM_CALIBRATION,
    ITEM_EXIT,
};
/*---------------------------------------------------------------------------*/
static item_t gs_cali_items[] = {
    item(ITEM_CLEAR,  uistr_info_sensor_cali_clear),
    item(ITEM_CALIBRATION,   uistr_info_sensor_cali_start),
    item(ITEM_EXIT,   uistr_info_sensor_back),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
enum{
    GS_OP_NONE,
    GS_OP_CLEAR,
    GS_OP_CALI_PRE,
    GS_OP_CALI,
}; 
/*---------------------------------------------------------------------------*/
#define C_MAX_MEASURE_NUM (20)

typedef struct
{
    int x;
    int y;
    int z;
	int valid;
}GS_CALI_DATA_STRUCT;

/*---------------------------------------------------------------------------*/
struct gsc_priv
{
    pthread_mutex_t evtmutex;
    /*specific data field*/
   int fd;
	char	*dev;

    int  pending_op;
    int  cali_delay;
    int  cali_num;
    int  cali_tolerance;
    bool bUpToDate; 
    HwmData cali_drv;
    HwmData cali_nvram;
    HwmData dat;

    char status[1024];
	GS_CALI_DATA_STRUCT gs_data;

    /*calculate statical information*/
    int statistics;  /*0: calculating; 1: done*/
    int measure_idx; 
    float raw[C_MAX_MEASURE_NUM][C_MAX_HWMSEN_EVENT_NUM];
    float std[C_MAX_HWMSEN_EVENT_NUM];
    float avg[C_MAX_HWMSEN_EVENT_NUM];
    float max[C_MAX_HWMSEN_EVENT_NUM];
    float min[C_MAX_HWMSEN_EVENT_NUM];
};
/*---------------------------------------------------------------------------*/
struct gsc_data
{
    struct gsc_priv gsc;

    /*common for each factory mode*/
    char  info[1024];
    bool  avail;
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int gs_cali_init_priv(struct gsc_priv *gs_cali)
{
    memset(gs_cali, 0x00, sizeof(*gs_cali));
    gs_cali->fd = -1;
    gs_cali->dev = "/dev/gsensor";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gs_cali_open(struct gsc_priv *gs_cali)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    if (gs_cali->fd == -1) {
        gs_cali->fd = open("/dev/gsensor", O_RDONLY);
        if (gs_cali->fd < 0) {
            LOGD("Couldn't open '%s' (%s)", gs_cali->dev, strerror(errno));
            return -1;
        }
        retry = 0;
        while ((err = ioctl(gs_cali->fd, GSENSOR_SET_ACC_MODE, &flags)) && (retry ++ < max_retry))
            usleep(retry_period*1000);
        if (err) {
            LOGD("enable gs fail: %s", strerror(errno));
            return -1;            
        } 
        retry = 0;
    }
    return 0;
}
static int gs_cali_close(struct gsc_priv *gs_cali)
{
    unsigned int flags = 0;
    int err;
    if (gs_cali->fd != -1) {
        if ((err = ioctl(gs_cali->fd, GSENSOR_SET_ACC_MODE, &flags))) {
            LOGD("disable gs fail: %s", strerror(errno));
            return -1;            
        } 
        close(gs_cali->fd);
    }
    memset(gs_cali, 0x00, sizeof(*gs_cali));
    gs_cali->fd = -1;
    gs_cali->dev = "/dev/gsensor";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int gs_cali_read_nv_data(struct gsc_priv *gs_cali)
{
	int file_lid = AP_CFG_RDCL_HWMON_ACC_WTCH_LID;    
    int rec_size;
    int rec_num, res;
	
	F_ID  fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);

	NVRAM_HWMON_ACC_WTCH_STRUCT hwmonGs =
	{	 
		{0, 0, 0, 0},
	};	   
  
	if(fd.iFileDesc < 0) 
	{
		NVRAM_LOG("nvram open = %d\n", fd.iFileDesc);
		return fd.iFileDesc;
	}
	
	res = read(fd.iFileDesc, &hwmonGs, rec_size*rec_num); 
	if(res < 0)
	{
		NVRAM_LOG("nvram read = %d(%s)\n", errno, strerror(errno));
	}
	else
	{
		gs_cali->gs_data.valid = hwmonGs.valid;
	}
	
	NVM_CloseFileDesc(fd); 
	
    return 0;
}
static int gs_cali_update_info(struct gsc_priv *gs_cali)
{
	 int err = -EINVAL, num = 0;
	 SENSOR_DATA  gs_dat;
	 int x, y, z;
	 static char buf[256];
	 
	 if (gs_cali->fd == -1){
		 LOGD("invalid fd\n");
		 return -EINVAL;
	 }
	 else{
	 		err = ioctl(gs_cali->fd, GSENSOR_IOCTL_READ_RAW_DATA, &gs_dat);
			if(err) 
			{
				 LOGD("read gs  raw: %d(%s)\n", err, strerror(errno));
				 LOGD(TAG "%s: Enter 2\n", __FUNCTION__);  
				 return err;
			}
			else
			{
				gs_cali->dat.rx = gs_dat.x;
				gs_cali->dat.ry = gs_dat.y;
				gs_cali->dat.rz = gs_dat.z;
			}
			
			err = ioctl(gs_cali->fd, GSENSOR_IOCTL_READ_SENSORDATA, buf);
			if(err)
			{
				LOGE("read data fail: %s(%d)\n", strerror(errno), errno);
			}
			else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
			{
				LOGE("read format fail: %s(%d)\n", strerror(errno), errno);
			}
			else
			{
				gs_cali->cali_drv.x = x ;
				gs_cali->cali_drv.y = y ;
				gs_cali->cali_drv.z = z ;
			}
		}
	 return 0;
}

/*---------------------------------------------------------------------------*/
static int gs_calibration(struct gsc_priv *gs_cali)
{
	int err = -EINVAL;
	SENSOR_DATA gs_raw_data={0,0,0},acc_sum={0,0,0};
	int acc_raw[20][3]={0}, acc_avg[3]={0};
	int count = 20;
	int i=0, j=0, offset=0;
	int cali_tolerance = 20; // 20%
	int offset_tolerance = 0;
	
	 if(gs_cali->fd == -1){
		 LOGD("invalid fd\n");
		 return -EINVAL;
	 }
	 else{
		for(i=0; i<count; i++)
		{
			if(err = ioctl(gs_cali->fd, GSENSOR_IOCTL_READ_RAW_DATA, &gs_raw_data))
			{
				LOGD("read gs raw: %d(%s)\n", err, strerror(errno));
				return err;
			}		
			acc_sum.x += gs_raw_data.x;
			acc_sum.y += gs_raw_data.y;
			acc_sum.z += gs_raw_data.z;
			
			acc_raw[i][0] = gs_raw_data.x;
			acc_raw[i][1] = gs_raw_data.y;
			acc_raw[i][2] = gs_raw_data.z;
			
			usleep(50000);
		}

			acc_avg[0] = acc_sum.x / count;
			acc_avg[1] = acc_sum.y / count;
			acc_avg[2] = acc_sum.z / count;
			
			offset_tolerance = GRAVITY_EARTH_1000 * cali_tolerance / 100;

		for(j=0; j<3; j++)
		{
			for(i=0;i<count;i++)
			{
				offset = abs(acc_raw[i][j]- acc_avg[j]);
				if(offset > offset_tolerance){
					return -EFAULT;
				}
			}
		}
		gs_cali->gs_data.x = acc_avg[0];
		gs_cali->gs_data.y = acc_avg[1];
		gs_cali->gs_data.z = acc_avg[2] - GRAVITY_EARTH_1000;

	    if((abs(gs_cali->gs_data.x) < 5000)
         &&(abs(gs_cali->gs_data.y) < 5000)
		 &&(abs(gs_cali->gs_data.z) < 5000))
		{
			gs_cali->gs_data.valid = 1;
			return 0;
		}
		else
		{
			gs_cali->gs_data.valid = 0;
			gs_cali->gs_data.x = 0;
			gs_cali->gs_data.y = 0;
			gs_cali->gs_data.z = 0;
			return -EINVAL;
		}
	 }
}
static int gs_write_nvram(struct gsc_priv *gs_cali)
{	
	int file_lid = AP_CFG_RDCL_HWMON_ACC_WTCH_LID;    
	int rec_size;
	int rec_num;
	int res;
	F_ID fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	
	NVRAM_HWMON_ACC_WTCH_STRUCT hwmonGs =
	{	 
		{0, 0, 0, 0},
	};	  
	if(fd.iFileDesc < 0)//fd.iFileDesc added by fangliang for build  [fd before]
	{
		LOGD("nvram open = %d\n", fd.iFileDesc);
		return fd.iFileDesc;
	}
	
	hwmonGs.x = gs_cali->gs_data.x;
	hwmonGs.y = gs_cali->gs_data.y;
	hwmonGs.z = gs_cali->gs_data.z;
	hwmonGs.valid = gs_cali->gs_data.valid;
	
	 res = write(fd.iFileDesc, &hwmonGs , rec_size*rec_num); 
	 if(res < 0)
	 {
		 printf("hongfu error %s\n",__func__);
		 return res;
	 }
	 
	 NVM_CloseFileDesc(fd); 
	 FileOp_BackupToBinRegion_All();  //backup TO BinRegion  Api 
	 sync();

	 usleep(50000);
	 property_set("ctl.start", "nvram_acc");
		 
	 return 0;
}
/*---------------------------------------------------------------------------*/
static void *gs_cali_update_iv_thread(void *priv)
{
	struct gsc_data *dat = (struct gsc_data *)priv; 
    struct gsc_priv *lgs = &dat->gsc;
	struct itemview *iv = dat->iv;
	int temp_valid = 0;
	int err = 0, len = 0;
	char *status;
	static int op = -1;

	LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = gs_cali_open(lgs))) {
		
		memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
		iv->redraw(iv);
        LOGD("gsensor() err = %d(%s)\n", err, dat->info);
		pthread_exit(NULL);
		return NULL;
	}
	if (err = gs_cali_read_nv_data(lgs)){
		LOGD("gsensor read_nv_data err = %d(%s)\n", err);
		pthread_exit(NULL);
		return NULL;
	}

    while (1) {

		if(dat->exit_thd)
		{
			break;
		}

		pthread_mutex_lock(&dat->gsc.evtmutex);
		if(op != dat->gsc.pending_op)
		{
			op = dat->gsc.pending_op;
			   LOGD("op: %d\n", dat->gsc.pending_op);
		}
		pthread_mutex_unlock(&dat->gsc.evtmutex);
		err = 0;

		if(op == GS_OP_CLEAR)
		{
			temp_valid=lgs->gs_data.valid;
			memset(&dat->gsc.gs_data, 0x00, sizeof(dat->gsc.gs_data));
			memset(&lgs->gs_data, 0x00, sizeof(lgs->gs_data));
			lgs->gs_data.valid = 0;
			if(err = gs_write_nvram(lgs))
			{
					lgs->gs_data.valid=temp_valid;
					LOGD("gs_write_nvram: %d\n", err); 
			}
			if(err)
			{
				len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), "Clear: Fail\n");  
				//dat->mod->test_result = FTM_TEST_FAIL;
			}
			else
			{
				len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), "Clear: Pass\n");
				//dat->mod->test_result = FTM_TEST_PASS;
			}
			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_NONE;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}
		else if(op == GS_OP_CALI_PRE)
		{
			err = 0;
			/*by-pass*/
		    snprintf(dat->gsc.status, sizeof(dat->gsc.status), "Calibrating: don't touch\n");	
			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_CALI;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}
		else if(op == GS_OP_CALI)
		{
			temp_valid = lgs->gs_data.valid;
            if(err = gs_calibration(lgs))
			{    
				lgs->gs_data.valid = temp_valid;
                LOGD("calibrate gs: %d\n", err); 
			}
            else if(err = gs_write_nvram(lgs))
			{
				lgs->gs_data.valid = temp_valid;
                LOGD("gs_write_nvram: %d\n", err); 
			}
			if(err)
			{
                len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), "Calibrating: Fail\n");
				dat->mod->test_result = FTM_TEST_FAIL;
			}
			else
			{
                len = snprintf(dat->gsc.status, sizeof(dat->gsc.status), "Calibrating: Pass\n");
				dat->mod->test_result = FTM_TEST_PASS;
			}
			
			pthread_mutex_lock(&dat->gsc.evtmutex);
			dat->gsc.pending_op = GS_OP_NONE;
			pthread_mutex_unlock(&dat->gsc.evtmutex);
		}

        if(err = gs_cali_update_info(lgs))
		{
            LOGD("gs_cali_update_info() = (%s), %d\n", strerror(errno), err);
			break;
		} 

		len = 0;
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "RawX: %+7.4f\nRawY: %+7.4f\nRawZ: %+7.4f\nCaliX: %+7.4f\nCaliY: %+7.4f\nCaliZ: %+7.4f\n", 
			((float)dat->gsc.dat.rx)/(1000.0), ((float)dat->gsc.dat.ry)/(1000.0), ((float)dat->gsc.dat.rz)/(1000.0),
			((float)dat->gsc.cali_drv.x)/(1000.0), ((float)dat->gsc.cali_drv.y)/(1000.0), ((float)dat->gsc.cali_drv.z)/(1000.0));
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "\nifcali %s\n\n\n", (dat->gsc.gs_data.valid == 1)?"yes":"no");
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s\n\n\n", dat->gsc.status);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s\n", uistr_info_sensor_cali_hint);		
		        
		iv->set_text(iv, &dat->text);
		iv->redraw(iv);
	}
	
    gs_cali_close(lgs);
	LOGD(TAG "%s: Exit\n", __FUNCTION__);    
	pthread_exit(NULL);

	return NULL;
}
/*---------------------------------------------------------------------------*/
int gs_cali_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct gsc_data *dat = (struct gsc_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err, op;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);
       
    snprintf(dat->info, sizeof(dat->info), uistr_info_sensor_initializing);
    dat->exit_thd = false;  


    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        dat->iv = iv;
    }
    iv = dat->iv;
    iv->set_title(iv, &dat->title);
    iv->set_items(iv, gs_cali_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, gs_cali_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        pthread_mutex_lock(&dat->gsc.evtmutex);
        op = dat->gsc.pending_op;
        pthread_mutex_unlock(&dat->gsc.evtmutex);        
        if ((chosen != ITEM_EXIT) && (op != GS_OP_NONE))   /*some OP is pending*/
            continue;
        switch (chosen) {
        case ITEM_CLEAR:
            pthread_mutex_lock(&dat->gsc.evtmutex);
            dat->gsc.pending_op = GS_OP_CLEAR;
            LOGD("chosen clear: %d\n",  dat->gsc.pending_op);
            pthread_mutex_unlock(&dat->gsc.evtmutex);
            break;
        case ITEM_CALIBRATION:
            pthread_mutex_lock(&dat->gsc.evtmutex);            
            dat->gsc.pending_op = GS_OP_CALI_PRE;
            LOGD("chosen Do calibration\n");
            pthread_mutex_unlock(&dat->gsc.evtmutex);
            break;
        case ITEM_EXIT:
            exit = true;            
            break;
        }
        
        if (exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int gs_cali_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct gsc_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_GS_CALI, sizeof(struct gsc_data));
    dat  = mod_to_gsc_data(mod);

    memset(dat, 0x00, sizeof(*dat));
	dat->gsc.fd = -1;
    gs_cali_init_priv(&dat->gsc);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, gs_cali_entry, (void*)dat);
//    if (!ret)
//        mod->visible = false;

    return ret;
}
#endif 
