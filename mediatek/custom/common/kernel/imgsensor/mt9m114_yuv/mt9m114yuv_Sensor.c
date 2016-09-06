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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mt9m114yuv_sensor.c
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 * 
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "mt9m114yuv_Sensor.h"
#include "mt9m114yuv_Camera_Sensor_para.h"
#include "mt9m114yuv_CameraCustomized.h"

#define mt9m114yuv_DEBUG
#ifdef mt9m114yuv_DEBUG
	#define SENSORDB(fmt, arg...) printk( "[mt9m114yuv] "  fmt, ##arg)
#else
	#define SENSORDB(x,...)
#endif


typedef enum {
    SENSOR_MODE_INIT = 0,
    SENSOR_MODE_PREVIEW,
    SENSOR_MODE_CAPTURE
}MT9M114_SENSOR_MODE;

typedef struct {
    kal_uint16 pvPCLK; // x10 240 for 24 MHZ 
    kal_uint16 capPCLK; // x10

    kal_uint16 iFrameRate;
    kal_uint16 iNightMode;
    kal_uint16 iWB;
    kal_uint16 iEffect;
    kal_uint16 iEV;
    kal_uint16 iBanding;
    kal_uint16 iMirror;
#if 1 //add by yujianhua
	kal_uint16 iIso;
	kal_uint16 iedge;
	kal_uint16 iSaturation;
	kal_uint16 iContrast;
	kal_uint16 ihue;
	kal_uint16 ibrightness;
#endif
    MT9M114_SENSOR_MODE sensorMode;
}MT9M114_PARA_STRUCT, *PMT9M114_PARA_STRUCT; 


MSDK_SCENARIO_ID_ENUM MT9M114_CurrentScenarioId = MSDK_SCENARIO_ID_CAMERA_PREVIEW;
MT9M114_PARA_STRUCT MT9M114_para;
MSDK_SENSOR_CONFIG_STRUCT MT9M114_SensorConfigData;

static DEFINE_SPINLOCK(mt9m114yuv_drv_lock);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 MT9M114_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte = 0;
    char puSendCmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF)};
    iReadRegI2C(puSendCmd, 2, (u8*)&get_byte, 2, MT9M114_READ_ID);
    return ((get_byte << 8) & 0xFF00) | ((get_byte >> 8) & 0x00FF);
}

void MT9M114_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 4, MT9M114_WRITE_ID);
}

kal_uint16 MT9M114_read_cmos_sensor_8(kal_uint32 addr)
{
    kal_uint16 get_byte = 0;
    char puSendCmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF)};
    iReadRegI2C(puSendCmd, 2, (u8*)&get_byte, 1, MT9M114_READ_ID);
    return get_byte;
}

void MT9M114_write_cmos_sensor_8(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 3, MT9M114_WRITE_ID);
}

void MT9M114_change_config_command()
{
    kal_uint16 command_register_value, time = 0;
    kal_uint16 change_config_shift = 1, change_config_mask = 0x0002;
    kal_uint16 command_ok_shift = 15, command_ok_mask = 0x8000;

    command_register_value = MT9M114_read_cmos_sensor(0x0080);
    
    if ((command_register_value & change_config_mask) >> change_config_shift != 0)
    {
        SENSORDB("[Error]: State change cmd bit is already set!! \n");
        return;
    }

    MT9M114_write_cmos_sensor(0x098E, 0xDC00);		// LOGICAL_ADDRESS_ACCESS
    MT9M114_write_cmos_sensor_8(0xDC00, 0x28);		// sysmgr_next_state
    MT9M114_write_cmos_sensor(0x0080, 0x8002);		// COMMAND_REGISTER

    // wait HOST_COMMAND_1
    time = 0;
    do {
        command_register_value = MT9M114_read_cmos_sensor(0x0080);
        if ((command_register_value & change_config_mask) >> change_config_shift == 0)
            break;
        mDELAY(10);
        time++;
    } while (time < 10);
	
	mDELAY(150);

	// check HOST_COMMAND_OK
    if ((command_register_value & command_ok_mask) >> command_ok_shift != 1)
    {
        SENSORDB("[Error]: Change-Config failed!! \n");
        return;
    }
}

void MT9M114_wait_command_0()
{
    kal_uint16 command_register_value, time = 0;

    // wait HOST_COMMAND_0
    time = 0;
    do {
        command_register_value = MT9M114_read_cmos_sensor(0x0080);

        // time out 100 ms
        time += 10;
        if (time > 100)
            break;
            
        mDELAY(10);
    } while (command_register_value & 0x0001 != 0x0000);
}



kal_uint16 MT9M114GetSensorID(void)
{
    kal_uint16 sensor_id = 0xFFFF;

    sensor_id = MT9M114_read_cmos_sensor(0x0000);
	printk("jin MT9M114GetSensorID  sensor_id=0x%x\n",sensor_id);
    return sensor_id;
}


/*************************************************************************
* FUNCTION
*   MT9M114CheckSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114CheckSensorID(UINT32 *sensorID)
{
    int  retry = 3;

    //SENSORDB("[Enter]: %s \n", __FUNCTION__);
    
    // check if sensor ID correct
    do {
        *sensorID = MT9M114GetSensorID();
        
        if (*sensorID == MT9M114_SENSOR_ID)
        {
            SENSORDB("yulianfeng :  MT9M114CheckSensorID SensorID = 0x%04x \n", __FUNCTION__, *sensorID);
            break; 
        }
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);
	
    //*sensorID =0x2481;
    if (*sensorID != MT9M114_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    //SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
    return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	MT9M114InitialPara
*
* DESCRIPTION
*	This function initialize the global status of  MT9M114
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void MT9M114InitialPara(void)
{
  /*Initial status setting 
   Can be better by sync with MT9M114InitialSetting*/
  spin_lock(&mt9m114yuv_drv_lock);
  MT9M114_para.iNightMode = 0xFFFF;
  MT9M114_para.iWB = AWB_MODE_AUTO;
  MT9M114_para.iEffect = MEFFECT_OFF;
  MT9M114_para.iBanding = AE_FLICKER_MODE_50HZ;
  MT9M114_para.iEV = AE_EV_COMP_00;
  MT9M114_para.iMirror = IMAGE_NORMAL;
  MT9M114_para.iFrameRate = 0; //No Fix FrameRate

  MT9M114_para.sensorMode = SENSOR_MODE_INIT;
  spin_unlock(&mt9m114yuv_drv_lock);
}


/*************************************************************************
* FUNCTION
*   MT9M114InitialSetting
*
* DESCRIPTION
*   This function set the sensor initial register setting.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114InitialSetting()
{
	//printk("jin MT9M114InitialSetting enter\n");
 	// soft reset
 
    MT9M114_write_cmos_sensor(0x001A, 0x0001);     // RESET_AND_MISC_CONTROL
    MT9M114_write_cmos_sensor(0x001A, 0x0000);     // RESET_AND_MISC_CONTROL

	MT9M114_write_cmos_sensor(0x301A, 0x8230); 	// RESET_REGISTER
	//DELAY=5
	mDELAY(50);
	MT9M114_write_cmos_sensor(0x098E, 0x1000); 	// LOGICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor_8(0xC97E, 0x01); 	// CAM_SYSCTL_PLL_ENABLE
	mDELAY(5); // add 2012-4-1 13:45
	MT9M114_write_cmos_sensor(0xC980, 0x033b); 	// CAM_SYSCTL_PLL_DIVIDER_M_N
	MT9M114_write_cmos_sensor(0xC982, 0x0700); 	// CAM_SYSCTL_PLL_DIVIDER_P

	MT9M114_write_cmos_sensor(0xC984, 0x8000);		//cam_port_output_control = 32768
	//DELAY=5
	mDELAY(10);

	//MT9M114_write_cmos_sensor( 0x098E, 0x0000);    // LOGICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor( 0xC800, 0x0004);    // CAM_SENSOR_CFG_Y_ADDR_START
	MT9M114_write_cmos_sensor( 0xC802, 0x0004);    // CAM_SENSOR_CFG_X_ADDR_START
	MT9M114_write_cmos_sensor( 0xC804, 0x03cb);    // CAM_SENSOR_CFG_Y_ADDR_END
	MT9M114_write_cmos_sensor( 0xC806, 0x050B);    // CAM_SENSOR_CFG_X_ADDR_END
	//MT9M114_write_cmos_sensor( 0xC808, 0x02db);    // CAM_SENSOR_CFG_PIXCLK
	//MT9M114_write_cmos_sensor( 0xC809, 0x77dc);    // CAM_SENSOR_CFG_PIXCLK
	//MT9M114_write_cmos_sensor( 0xC80A, 0x77dc);    // CAM_SENSOR_CFG_PIXCLK
	MT9M114_write_cmos_sensor_8(0xC808, 0x02);
	MT9M114_write_cmos_sensor_8(0xC809, 0xDB);
	MT9M114_write_cmos_sensor_8(0xC80A, 0x77);
	MT9M114_write_cmos_sensor_8(0xC80B, 0xDC);

	MT9M114_write_cmos_sensor( 0xC80C, 0x0001);    // CAM_SENSOR_CFG_ROW_SPEED
	MT9M114_write_cmos_sensor( 0xC80E, 0x00DB);    // CAM_SENSOR_CFG_FINE_INTEG_TIME_MIN
	MT9M114_write_cmos_sensor( 0xC810, 0x05c7);    // CAM_SENSOR_CFG_FINE_INTEG_TIME_MAX
	MT9M114_write_cmos_sensor( 0xC812, 0x03EE);    // CAM_SENSOR_CFG_FRAME_LENGTH_LINES
	MT9M114_write_cmos_sensor( 0xC814, 0x064a);    // CAM_SENSOR_CFG_LINE_LENGTH_PCK
	MT9M114_write_cmos_sensor( 0xC816, 0x0060);    // CAM_SENSOR_CFG_FINE_CORRECTION
	MT9M114_write_cmos_sensor( 0xC818, 0x03c3);    // CAM_SENSOR_CFG_CPIPE_LAST_ROW
	MT9M114_write_cmos_sensor( 0xC834, 0x0000);    // CAM_SENSOR_CONTROL_READ_MODE

	//MT9M114_write_cmos_sensor( 0xC88C, 0x1d99);    // CAM_AET_MAX_FRAME_RATE
	//MT9M114_write_cmos_sensor( 0xC88E, 0x0500);    // CAM_AET_MIN_FRAME_RATE

	// select output size, YUV format
	// 1280x960
	MT9M114_write_cmos_sensor( 0xC868, 0x0500);    // CAM_OUTPUT_WIDTH
	MT9M114_write_cmos_sensor( 0xC86A, 0x03c0);    // CAM_OUTPUT_HEIGHT
	
	
	MT9M114_write_cmos_sensor_8( 0xC85C, 0x03);    // CAM_CROP_CROPMODE
	MT9M114_write_cmos_sensor( 0xC854, 0x0000);    // CAM_CROP_WINDOW_XOFFSET
	MT9M114_write_cmos_sensor( 0xC856, 0x0000);    // CAM_CROP_WINDOW_YOFFSET
	MT9M114_write_cmos_sensor( 0xC858, 0x0500);    // CAM_CROP_WINDOW_WIDTH
    MT9M114_write_cmos_sensor( 0xC85A, 0x03C0);    // CAM_CROP_WINDOW_HEIGHT
    
	
	MT9M114_write_cmos_sensor( 0xC914, 0x0000);    // CAM_STAT_AWB_CLIP_WINDOW_XSTART
	MT9M114_write_cmos_sensor( 0xC916, 0x0000);    // CAM_STAT_AWB_CLIP_WINDOW_YSTART
	MT9M114_write_cmos_sensor( 0xC918, 0x04FF);    // CAM_STAT_AWB_CLIP_WINDOW_XEND
	MT9M114_write_cmos_sensor( 0xC91A, 0x02CF);    // CAM_STAT_AWB_CLIP_WINDOW_YEND
	MT9M114_write_cmos_sensor( 0xC91C, 0x0000);    // CAM_STAT_AE_INITIAL_WINDOW_XSTART
	MT9M114_write_cmos_sensor( 0xC91E, 0x0000);    // CAM_STAT_AE_INITIAL_WINDOW_YSTART
	MT9M114_write_cmos_sensor( 0xC920, 0x00FF);    // CAM_STAT_AE_INITIAL_WINDOW_XEND
	MT9M114_write_cmos_sensor( 0xC922, 0x008F);    // CAM_STAT_AE_INITIAL_WINDOW_YEND

	MT9M114_write_cmos_sensor_8( 0xE801, 0x00);    // AUTO_BINNING_MODE

	//[Step3-Recommended]

	MT9M114_write_cmos_sensor( 0x316A, 0x8270);    // DAC_TXLO_ROW
	MT9M114_write_cmos_sensor( 0x316C, 0x8270);    // DAC_TXLO
	MT9M114_write_cmos_sensor( 0x3ED0, 0x2305);    // DAC_LD_4_5
	MT9M114_write_cmos_sensor( 0x3ED2, 0x77CF);    // DAC_LD_6_7
	MT9M114_write_cmos_sensor( 0x316E, 0x8202);    // DAC_ECL
	MT9M114_write_cmos_sensor( 0x3180, 0x87FF);    // DELTA_DK_CONTROL
	MT9M114_write_cmos_sensor( 0x30D4, 0x6080);    // COLUMN_CORRECTION
	MT9M114_write_cmos_sensor( 0xA802, 0x0008);    // AE_TRACK_MODE
	MT9M114_write_cmos_sensor( 0x3E14, 0xFF39);    // SAMP_COL_PUP2

	//[patch 1204]for 
	MT9M114_write_cmos_sensor( 0x0982, 0x0001);    // ACCESS_CTL_STAT
	MT9M114_write_cmos_sensor( 0x098A, 0x60BC);    // PHYSICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor( 0xE0BC, 0xC0F1);
	MT9M114_write_cmos_sensor( 0xE0BE, 0x082A);
	MT9M114_write_cmos_sensor( 0xE0C0, 0x05A0);
	MT9M114_write_cmos_sensor( 0xE0C2, 0xD800);
	MT9M114_write_cmos_sensor( 0xE0C4, 0x71CF);
	MT9M114_write_cmos_sensor( 0xE0C6, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE0C8, 0xC344);
	MT9M114_write_cmos_sensor( 0xE0CA, 0x77CF);
	MT9M114_write_cmos_sensor( 0xE0CC, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE0CE, 0xC7C0);
	MT9M114_write_cmos_sensor( 0xE0D0, 0xB104);
	MT9M114_write_cmos_sensor( 0xE0D2, 0x8F1F);
	MT9M114_write_cmos_sensor( 0xE0D4, 0x75CF);
	MT9M114_write_cmos_sensor( 0xE0D6, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE0D8, 0xC84C);
	MT9M114_write_cmos_sensor( 0xE0DA, 0x0811);
	MT9M114_write_cmos_sensor( 0xE0DC, 0x005E);
	MT9M114_write_cmos_sensor( 0xE0DE, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE0E0, 0x0000);
	MT9M114_write_cmos_sensor( 0xE0E2, 0x500E);
	MT9M114_write_cmos_sensor( 0xE0E4, 0x7840);
	MT9M114_write_cmos_sensor( 0xE0E6, 0xF019);
	MT9M114_write_cmos_sensor( 0xE0E8, 0x0CC6);
	MT9M114_write_cmos_sensor( 0xE0EA, 0x0340);
	MT9M114_write_cmos_sensor( 0xE0EC, 0x0E26);
	MT9M114_write_cmos_sensor( 0xE0EE, 0x0340);
	MT9M114_write_cmos_sensor( 0xE0F0, 0x95C2);
	MT9M114_write_cmos_sensor( 0xE0F2, 0x0E21);
	MT9M114_write_cmos_sensor( 0xE0F4, 0x101E);
	MT9M114_write_cmos_sensor( 0xE0F6, 0x0E0D);
	MT9M114_write_cmos_sensor( 0xE0F8, 0x119E);
	MT9M114_write_cmos_sensor( 0xE0FA, 0x0D56);
	MT9M114_write_cmos_sensor( 0xE0FC, 0x0340);
	MT9M114_write_cmos_sensor( 0xE0FE, 0xF008);
	MT9M114_write_cmos_sensor( 0xE100, 0x2650);
	MT9M114_write_cmos_sensor( 0xE102, 0x1040);
	MT9M114_write_cmos_sensor( 0xE104, 0x0AA2);
	MT9M114_write_cmos_sensor( 0xE106, 0x0360);
	MT9M114_write_cmos_sensor( 0xE108, 0xB502);
	MT9M114_write_cmos_sensor( 0xE10A, 0xB5C2);
	MT9M114_write_cmos_sensor( 0xE10C, 0x0B22);
	MT9M114_write_cmos_sensor( 0xE10E, 0x0400);
	MT9M114_write_cmos_sensor( 0xE110, 0x0CCE);
	MT9M114_write_cmos_sensor( 0xE112, 0x0320);
	MT9M114_write_cmos_sensor( 0xE114, 0xD800);
	MT9M114_write_cmos_sensor( 0xE116, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE118, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE11A, 0xC5D4);
	MT9M114_write_cmos_sensor( 0xE11C, 0x902C);
	MT9M114_write_cmos_sensor( 0xE11E, 0x72CF);
	MT9M114_write_cmos_sensor( 0xE120, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE122, 0xE218);
	MT9M114_write_cmos_sensor( 0xE124, 0x9009);
	MT9M114_write_cmos_sensor( 0xE126, 0xE105);
	MT9M114_write_cmos_sensor( 0xE128, 0x73CF);
	MT9M114_write_cmos_sensor( 0xE12A, 0xFF00);
	MT9M114_write_cmos_sensor( 0xE12C, 0x2FD0);
	MT9M114_write_cmos_sensor( 0xE12E, 0x7822);
	MT9M114_write_cmos_sensor( 0xE130, 0x7910);
	MT9M114_write_cmos_sensor( 0xE132, 0xB202);
	MT9M114_write_cmos_sensor( 0xE134, 0x1382);
	MT9M114_write_cmos_sensor( 0xE136, 0x0700);
	MT9M114_write_cmos_sensor( 0xE138, 0x0815);
	MT9M114_write_cmos_sensor( 0xE13A, 0x03DE);
	MT9M114_write_cmos_sensor( 0xE13C, 0x1387);
	MT9M114_write_cmos_sensor( 0xE13E, 0x0700);
	MT9M114_write_cmos_sensor( 0xE140, 0x2102);
	MT9M114_write_cmos_sensor( 0xE142, 0x000A);
	MT9M114_write_cmos_sensor( 0xE144, 0x212F);
	MT9M114_write_cmos_sensor( 0xE146, 0x0288);
	MT9M114_write_cmos_sensor( 0xE148, 0x1A04);
	MT9M114_write_cmos_sensor( 0xE14A, 0x0284);
	MT9M114_write_cmos_sensor( 0xE14C, 0x13B9);
	MT9M114_write_cmos_sensor( 0xE14E, 0x0700);
	MT9M114_write_cmos_sensor( 0xE150, 0xB8C1);
	MT9M114_write_cmos_sensor( 0xE152, 0x0815);
	MT9M114_write_cmos_sensor( 0xE154, 0x0052);
	MT9M114_write_cmos_sensor( 0xE156, 0xDB00);
	MT9M114_write_cmos_sensor( 0xE158, 0x230F);
	MT9M114_write_cmos_sensor( 0xE15A, 0x0003);
	MT9M114_write_cmos_sensor( 0xE15C, 0x2102);
	MT9M114_write_cmos_sensor( 0xE15E, 0x00C0);
	MT9M114_write_cmos_sensor( 0xE160, 0x7910);
	MT9M114_write_cmos_sensor( 0xE162, 0xB202);
	MT9M114_write_cmos_sensor( 0xE164, 0x9507);
	MT9M114_write_cmos_sensor( 0xE166, 0x7822);
	MT9M114_write_cmos_sensor( 0xE168, 0xE080);
	MT9M114_write_cmos_sensor( 0xE16A, 0xD900);
	MT9M114_write_cmos_sensor( 0xE16C, 0x20CA);
	MT9M114_write_cmos_sensor( 0xE16E, 0x004B);
	MT9M114_write_cmos_sensor( 0xE170, 0xB805);
	MT9M114_write_cmos_sensor( 0xE172, 0x9533);
	MT9M114_write_cmos_sensor( 0xE174, 0x7815);
	MT9M114_write_cmos_sensor( 0xE176, 0x6038);
	MT9M114_write_cmos_sensor( 0xE178, 0x0FB2);
	MT9M114_write_cmos_sensor( 0xE17A, 0x0560);
	MT9M114_write_cmos_sensor( 0xE17C, 0xB861);
	MT9M114_write_cmos_sensor( 0xE17E, 0xB711);
	MT9M114_write_cmos_sensor( 0xE180, 0x0775);
	MT9M114_write_cmos_sensor( 0xE182, 0x0540);
	MT9M114_write_cmos_sensor( 0xE184, 0xD900);
	MT9M114_write_cmos_sensor( 0xE186, 0xF00A);
	MT9M114_write_cmos_sensor( 0xE188, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE18A, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE18C, 0xE210);
	MT9M114_write_cmos_sensor( 0xE18E, 0x7835);
	MT9M114_write_cmos_sensor( 0xE190, 0x8041);
	MT9M114_write_cmos_sensor( 0xE192, 0x8000);
	MT9M114_write_cmos_sensor( 0xE194, 0xE102);
	MT9M114_write_cmos_sensor( 0xE196, 0xA040);
	MT9M114_write_cmos_sensor( 0xE198, 0x09F1);
	MT9M114_write_cmos_sensor( 0xE19A, 0x8094);
	MT9M114_write_cmos_sensor( 0xE19C, 0x7FE0);
	MT9M114_write_cmos_sensor( 0xE19E, 0xD800);
	MT9M114_write_cmos_sensor( 0xE1A0, 0xC0F1);
	MT9M114_write_cmos_sensor( 0xE1A2, 0xC5E1);
	MT9M114_write_cmos_sensor( 0xE1A4, 0x71CF);
	MT9M114_write_cmos_sensor( 0xE1A6, 0x0000);
	MT9M114_write_cmos_sensor( 0xE1A8, 0x45E6);
	MT9M114_write_cmos_sensor( 0xE1AA, 0x7960);
	MT9M114_write_cmos_sensor( 0xE1AC, 0x7508);
	MT9M114_write_cmos_sensor( 0xE1AE, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE1B0, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE1B2, 0xC84C);
	MT9M114_write_cmos_sensor( 0xE1B4, 0x9002);
	MT9M114_write_cmos_sensor( 0xE1B6, 0x083D);
	MT9M114_write_cmos_sensor( 0xE1B8, 0x021E);
	MT9M114_write_cmos_sensor( 0xE1BA, 0x0D39);
	MT9M114_write_cmos_sensor( 0xE1BC, 0x10D1);
	MT9M114_write_cmos_sensor( 0xE1BE, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE1C0, 0xFF00);
	MT9M114_write_cmos_sensor( 0xE1C2, 0x3354);
	MT9M114_write_cmos_sensor( 0xE1C4, 0x9055);
	MT9M114_write_cmos_sensor( 0xE1C6, 0x71CF);
	MT9M114_write_cmos_sensor( 0xE1C8, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE1CA, 0xC5D4);
	MT9M114_write_cmos_sensor( 0xE1CC, 0x116C);
	MT9M114_write_cmos_sensor( 0xE1CE, 0x0103);
	MT9M114_write_cmos_sensor( 0xE1D0, 0x1170);
	MT9M114_write_cmos_sensor( 0xE1D2, 0x00C1);
	MT9M114_write_cmos_sensor( 0xE1D4, 0xE381);
	MT9M114_write_cmos_sensor( 0xE1D6, 0x22C6);
	MT9M114_write_cmos_sensor( 0xE1D8, 0x0F81);
	MT9M114_write_cmos_sensor( 0xE1DA, 0x0000);
	MT9M114_write_cmos_sensor( 0xE1DC, 0x00FF);
	MT9M114_write_cmos_sensor( 0xE1DE, 0x22C4);
	MT9M114_write_cmos_sensor( 0xE1E0, 0x0F82);
	MT9M114_write_cmos_sensor( 0xE1E2, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE1E4, 0x00FF);
	MT9M114_write_cmos_sensor( 0xE1E6, 0x29C0);
	MT9M114_write_cmos_sensor( 0xE1E8, 0x0222);
	MT9M114_write_cmos_sensor( 0xE1EA, 0x7945);
	MT9M114_write_cmos_sensor( 0xE1EC, 0x7930);
	MT9M114_write_cmos_sensor( 0xE1EE, 0xB035);
	MT9M114_write_cmos_sensor( 0xE1F0, 0x0715);
	MT9M114_write_cmos_sensor( 0xE1F2, 0x0540);
	MT9M114_write_cmos_sensor( 0xE1F4, 0xD900);
	MT9M114_write_cmos_sensor( 0xE1F6, 0xF00A);
	MT9M114_write_cmos_sensor( 0xE1F8, 0x70CF);
	MT9M114_write_cmos_sensor( 0xE1FA, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE1FC, 0xE224);
	MT9M114_write_cmos_sensor( 0xE1FE, 0x7835);
	MT9M114_write_cmos_sensor( 0xE200, 0x8041);
	MT9M114_write_cmos_sensor( 0xE202, 0x8000);
	MT9M114_write_cmos_sensor( 0xE204, 0xE102);
	MT9M114_write_cmos_sensor( 0xE206, 0xA040);
	MT9M114_write_cmos_sensor( 0xE208, 0x09F1);
	MT9M114_write_cmos_sensor( 0xE20A, 0x8094);
	MT9M114_write_cmos_sensor( 0xE20C, 0x7FE0);
	MT9M114_write_cmos_sensor( 0xE20E, 0xD800);
	MT9M114_write_cmos_sensor( 0xE210, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE212, 0xCB40);
	MT9M114_write_cmos_sensor( 0xE214, 0xFFFF);
	MT9M114_write_cmos_sensor( 0xE216, 0xE0BC);
	MT9M114_write_cmos_sensor( 0xE218, 0x0000);
	MT9M114_write_cmos_sensor( 0xE21A, 0x0000);
	MT9M114_write_cmos_sensor( 0xE21C, 0x0000);
	MT9M114_write_cmos_sensor( 0xE21E, 0x0000);
	MT9M114_write_cmos_sensor( 0xE220, 0x0000);
	MT9M114_write_cmos_sensor( 0x098E, 0x0000);    // LOGICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor( 0xE000, 0x1184);    // PATCHLDR_LOADER_ADDRESS
	MT9M114_write_cmos_sensor( 0xE002, 0x1204);    // PATCHLDR_PATCH_ID
	//MT9M114_write_cmos_sensor( 0xE004, 0x41030202);    // PATCHLDR_FIRMWARE_ID
	MT9M114_write_cmos_sensor( 0xE004, 0x4103);
	MT9M114_write_cmos_sensor( 0xE006, 0x0202);
	MT9M114_write_cmos_sensor( 0x0080, 0xFFF0);    // COMMAND_REGISTER
	//  POLL  COMMAND_REGISTER::HOST_COMMAND_0 =>  0x00
	//  POLL  COMMAND_REGISTER::HOST_COMMAND_0 =>  0x00

	//  POLL  COMMAND_REGISTER::HOST_COMMAND_0 =>  0x00
	// 读Reg= 0x080， 判断其最低位是否为0， 如果不为0，则delay 5ms，然后继续读， 
	// 直到为0或者   50ms以上
	MT9M114_write_cmos_sensor( 0x0080, 0xFFF1);    // COMMAND_REGISTER
	mDELAY(50);
	// AWB Start point
	MT9M114_write_cmos_sensor(0x098E,0x2c12);
	//Reg = 0xac12,0x008f ///////////
	//Reg = 0xac14,0x0105


	//[Step4-APGA //LSC]
	MT9M114_write_cmos_sensor( 0x098E, 0x0000);    // LOGICAL_ADDRESS_ACCESS
	//MT9M114_write_cmos_sensor( 0xC95E, 0x0003);    // CAM_PGA_PGA_CONTROL
	MT9M114_write_cmos_sensor( 0xC95E, 0x0001);    // CAM_PGA_PGA_CONTROL
	//MT9M114_write_cmos_sensor( 0xC95E, 0x0002);    // CAM_PGA_PGA_CONTROL
	MT9M114_write_cmos_sensor( 0xC95E, 0x0001);    // CAM_PGA_PGA_CONTROL

MT9M114_write_cmos_sensor(0x098E, 0x495E);    // LOGICAL_ADDRESS_ACCESS [CAM_PGA_PGA_CONTROL]
MT9M114_write_cmos_sensor(0xC95E, 0x0000);    // CAM_PGA_PGA_CONTROL
MT9M114_write_cmos_sensor(0x3640, 0x0470);    // P_G1_P0Q0
MT9M114_write_cmos_sensor(0x3642, 0x828B);    // P_G1_P0Q1
MT9M114_write_cmos_sensor(0x3644, 0x0431);    // P_G1_P0Q2
MT9M114_write_cmos_sensor(0x3646, 0xB7EE);    // P_G1_P0Q3
MT9M114_write_cmos_sensor(0x3648, 0xD50F);    // P_G1_P0Q4
MT9M114_write_cmos_sensor(0x364A, 0x00B0);    // P_R_P0Q0
MT9M114_write_cmos_sensor(0x364C, 0xACCA);    // P_R_P0Q1
MT9M114_write_cmos_sensor(0x364E, 0x2151);    // P_R_P0Q2
MT9M114_write_cmos_sensor(0x3650, 0x920E);    // P_R_P0Q3
MT9M114_write_cmos_sensor(0x3652, 0xD24F);    // P_R_P0Q4
MT9M114_write_cmos_sensor(0x3654, 0x0270);    // P_B_P0Q0
MT9M114_write_cmos_sensor(0x3656, 0x6748);    // P_B_P0Q1
MT9M114_write_cmos_sensor(0x3658, 0x37D0);    // P_B_P0Q2
MT9M114_write_cmos_sensor(0x365A, 0xDC8D);    // P_B_P0Q3
MT9M114_write_cmos_sensor(0x365C, 0xE0AC);    // P_B_P0Q4
MT9M114_write_cmos_sensor(0x365E, 0x01F0);    // P_G2_P0Q0
MT9M114_write_cmos_sensor(0x3660, 0xC5CB);    // P_G2_P0Q1
MT9M114_write_cmos_sensor(0x3662, 0x02F1);    // P_G2_P0Q2
MT9M114_write_cmos_sensor(0x3664, 0xAFEE);    // P_G2_P0Q3
MT9M114_write_cmos_sensor(0x3666, 0xE66F);    // P_G2_P0Q4
MT9M114_write_cmos_sensor(0x3680, 0xF16B);    // P_G1_P1Q0
MT9M114_write_cmos_sensor(0x3682, 0xAA6C);    // P_G1_P1Q1
MT9M114_write_cmos_sensor(0x3684, 0x1F0F);    // P_G1_P1Q2
MT9M114_write_cmos_sensor(0x3686, 0x48EC);    // P_G1_P1Q3
MT9M114_write_cmos_sensor(0x3688, 0xAA0F);    // P_G1_P1Q4
MT9M114_write_cmos_sensor(0x368A, 0x808B);    // P_R_P1Q0
MT9M114_write_cmos_sensor(0x368C, 0xF1AA);    // P_R_P1Q1
MT9M114_write_cmos_sensor(0x368E, 0x666E);    // P_R_P1Q2
MT9M114_write_cmos_sensor(0x3690, 0x144D);    // P_R_P1Q3
MT9M114_write_cmos_sensor(0x3692, 0x92CF);    // P_R_P1Q4
MT9M114_write_cmos_sensor(0x3694, 0x0ECC);    // P_B_P1Q0
MT9M114_write_cmos_sensor(0x3696, 0x1CAA);    // P_B_P1Q1
MT9M114_write_cmos_sensor(0x3698, 0xA46D);    // P_B_P1Q2
MT9M114_write_cmos_sensor(0x369A, 0x1A0C);    // P_B_P1Q3
MT9M114_write_cmos_sensor(0x369C, 0x362C);    // P_B_P1Q4
MT9M114_write_cmos_sensor(0x369E, 0xC02A);    // P_G2_P1Q0
MT9M114_write_cmos_sensor(0x36A0, 0x13AC);    // P_G2_P1Q1
MT9M114_write_cmos_sensor(0x36A2, 0xF0CD);    // P_G2_P1Q2
MT9M114_write_cmos_sensor(0x36A4, 0xEB0D);    // P_G2_P1Q3
MT9M114_write_cmos_sensor(0x36A6, 0x1D6F);    // P_G2_P1Q4
MT9M114_write_cmos_sensor(0x36C0, 0x0491);    // P_G1_P2Q0
MT9M114_write_cmos_sensor(0x36C2, 0x9D6F);    // P_G1_P2Q1
MT9M114_write_cmos_sensor(0x36C4, 0x1851);    // P_G1_P2Q2
MT9M114_write_cmos_sensor(0x36C6, 0x2E30);    // P_G1_P2Q3
MT9M114_write_cmos_sensor(0x36C8, 0xB113);    // P_G1_P2Q4
MT9M114_write_cmos_sensor(0x36CA, 0x2831);    // P_R_P2Q0
MT9M114_write_cmos_sensor(0x36CC, 0xA5AD);    // P_R_P2Q1
MT9M114_write_cmos_sensor(0x36CE, 0x2772);    // P_R_P2Q2
MT9M114_write_cmos_sensor(0x36D0, 0x3C6F);    // P_R_P2Q3
MT9M114_write_cmos_sensor(0x36D2, 0xA9D4);    // P_R_P2Q4
MT9M114_write_cmos_sensor(0x36D4, 0x6710);    // P_B_P2Q0
MT9M114_write_cmos_sensor(0x36D6, 0x100B);    // P_B_P2Q1
MT9M114_write_cmos_sensor(0x36D8, 0x5B51);    // P_B_P2Q2
MT9M114_write_cmos_sensor(0x36DA, 0x49CE);    // P_B_P2Q3
MT9M114_write_cmos_sensor(0x36DC, 0xBEF3);    // P_B_P2Q4
MT9M114_write_cmos_sensor(0x36DE, 0x0811);    // P_G2_P2Q0
MT9M114_write_cmos_sensor(0x36E0, 0xA3EF);    // P_G2_P2Q1
MT9M114_write_cmos_sensor(0x36E2, 0x0F91);    // P_G2_P2Q2
MT9M114_write_cmos_sensor(0x36E4, 0x2C90);    // P_G2_P2Q3
MT9M114_write_cmos_sensor(0x36E6, 0xAC33);    // P_G2_P2Q4
MT9M114_write_cmos_sensor(0x3700, 0x308D);    // P_G1_P3Q0
MT9M114_write_cmos_sensor(0x3702, 0x1EAD);    // P_G1_P3Q1
MT9M114_write_cmos_sensor(0x3704, 0xA0B0);    // P_G1_P3Q2
MT9M114_write_cmos_sensor(0x3706, 0xA30D);    // P_G1_P3Q3
MT9M114_write_cmos_sensor(0x3708, 0xBACF);    // P_G1_P3Q4
MT9M114_write_cmos_sensor(0x370A, 0x918D);    // P_R_P3Q0
MT9M114_write_cmos_sensor(0x370C, 0x9D2D);    // P_R_P3Q1
MT9M114_write_cmos_sensor(0x370E, 0x312C);    // P_R_P3Q2
MT9M114_write_cmos_sensor(0x3710, 0x93AF);    // P_R_P3Q3
MT9M114_write_cmos_sensor(0x3712, 0xA550);    // P_R_P3Q4
MT9M114_write_cmos_sensor(0x3714, 0x9A2B);    // P_B_P3Q0
MT9M114_write_cmos_sensor(0x3716, 0x3E0B);    // P_B_P3Q1
MT9M114_write_cmos_sensor(0x3718, 0xFA4A);    // P_B_P3Q2
MT9M114_write_cmos_sensor(0x371A, 0xBAF0);    // P_B_P3Q3
MT9M114_write_cmos_sensor(0x371C, 0x51D0);    // P_B_P3Q4
MT9M114_write_cmos_sensor(0x371E, 0x0BAE);    // P_G2_P3Q0
MT9M114_write_cmos_sensor(0x3720, 0x9DCC);    // P_G2_P3Q1
MT9M114_write_cmos_sensor(0x3722, 0xBC4E);    // P_G2_P3Q2
MT9M114_write_cmos_sensor(0x3724, 0x586F);    // P_G2_P3Q3
MT9M114_write_cmos_sensor(0x3726, 0x9C8F);    // P_G2_P3Q4
MT9M114_write_cmos_sensor(0x3740, 0x594B);    // P_G1_P4Q0
MT9M114_write_cmos_sensor(0x3742, 0x4B30);    // P_G1_P4Q1
MT9M114_write_cmos_sensor(0x3744, 0x8115);    // P_G1_P4Q2
MT9M114_write_cmos_sensor(0x3746, 0xB252);    // P_G1_P4Q3
MT9M114_write_cmos_sensor(0x3748, 0x6476);    // P_G1_P4Q4
MT9M114_write_cmos_sensor(0x374A, 0x504E);    // P_R_P4Q0
MT9M114_write_cmos_sensor(0x374C, 0xEEEB);    // P_R_P4Q1
MT9M114_write_cmos_sensor(0x374E, 0xB6D5);    // P_R_P4Q2
MT9M114_write_cmos_sensor(0x3750, 0x9070);    // P_R_P4Q3
MT9M114_write_cmos_sensor(0x3752, 0x0E97);    // P_R_P4Q4
MT9M114_write_cmos_sensor(0x3754, 0x8B4F);    // P_B_P4Q0
MT9M114_write_cmos_sensor(0x3756, 0xCE0F);    // P_B_P4Q1
MT9M114_write_cmos_sensor(0x3758, 0xF754);    // P_B_P4Q2
MT9M114_write_cmos_sensor(0x375A, 0x830F);    // P_B_P4Q3
MT9M114_write_cmos_sensor(0x375C, 0x6996);    // P_B_P4Q4
MT9M114_write_cmos_sensor(0x375E, 0xF74B);    // P_G2_P4Q0
MT9M114_write_cmos_sensor(0x3760, 0x6710);    // P_G2_P4Q1
MT9M114_write_cmos_sensor(0x3762, 0x8115);    // P_G2_P4Q2
MT9M114_write_cmos_sensor(0x3764, 0xCB92);    // P_G2_P4Q3
MT9M114_write_cmos_sensor(0x3766, 0x6396);    // P_G2_P4Q4
MT9M114_write_cmos_sensor(0x3782, 0x01E0);    // CENTER_ROW
MT9M114_write_cmos_sensor(0x3784, 0x0280);    // CENTER_COLUMN
MT9M114_write_cmos_sensor(0x37C0, 0x810A);    // P_GR_Q5
MT9M114_write_cmos_sensor(0x37C2, 0x5828);    // P_RD_Q5
MT9M114_write_cmos_sensor(0x37C4, 0x8A2B);    // P_BL_Q5
MT9M114_write_cmos_sensor(0x37C6, 0xCD4A);    // P_GB_Q5
MT9M114_write_cmos_sensor(0xC960, 0x0000);    // CAM_PGA_L_CONFIG_COLOUR_TEMP
MT9M114_write_cmos_sensor(0xC962, 0x0000);    // CAM_PGA_L_CONFIG_GREEN_RED_Q14
MT9M114_write_cmos_sensor(0xC964, 0x0000);    // CAM_PGA_L_CONFIG_RED_Q14
MT9M114_write_cmos_sensor(0xC966, 0x0000);    // CAM_PGA_L_CONFIG_GREEN_BLUE_Q14
MT9M114_write_cmos_sensor(0xC968, 0x0000);    // CAM_PGA_L_CONFIG_BLUE_Q14
MT9M114_write_cmos_sensor(0xC96A, 0x0000);    // CAM_PGA_M_CONFIG_COLOUR_TEMP
MT9M114_write_cmos_sensor(0xC96C, 0x0000);    // CAM_PGA_M_CONFIG_GREEN_RED_Q14
MT9M114_write_cmos_sensor(0xC96E, 0x0000);    // CAM_PGA_M_CONFIG_RED_Q14
MT9M114_write_cmos_sensor(0xC970, 0x0000);    // CAM_PGA_M_CONFIG_GREEN_BLUE_Q14
MT9M114_write_cmos_sensor(0xC972, 0x0000);    // CAM_PGA_M_CONFIG_BLUE_Q14
MT9M114_write_cmos_sensor(0xC974, 0x0000);    // CAM_PGA_R_CONFIG_COLOUR_TEMP
MT9M114_write_cmos_sensor(0xC976, 0x0000);    // CAM_PGA_R_CONFIG_GREEN_RED_Q14
MT9M114_write_cmos_sensor(0xC978, 0x0000);    // CAM_PGA_R_CONFIG_RED_Q14
MT9M114_write_cmos_sensor(0xC97A, 0x0000);    // CAM_PGA_R_CONFIG_GREEN_BLUE_Q14
MT9M114_write_cmos_sensor(0xC97C, 0x0000);    // CAM_PGA_R_CONFIG_BLUE_Q14
MT9M114_write_cmos_sensor(0xC95E, 0x0000);    // CAM_PGA_PGA_CONTROL
MT9M114_write_cmos_sensor(0xC95E, 0x0001);    // CAM_PGA_PGA_CONTROL



	//[Step5-AWB_CCM]
	MT9M114_write_cmos_sensor( 0x098E, 0x4892);    // LOGICAL_ADDRESS_ACCESS [CAM_AWB_CCM_L_0]
	MT9M114_write_cmos_sensor( 0xC892, 0x0267);    // CAM_AWB_CCM_L_0
	MT9M114_write_cmos_sensor( 0xC894, 0xFF1A);    // CAM_AWB_CCM_L_1
	MT9M114_write_cmos_sensor( 0xC896, 0xFFB3);    // CAM_AWB_CCM_L_2
	MT9M114_write_cmos_sensor( 0xC898, 0xFF80);    // CAM_AWB_CCM_L_3
	MT9M114_write_cmos_sensor( 0xC89A, 0x0166);    // CAM_AWB_CCM_L_4
	MT9M114_write_cmos_sensor( 0xC89C, 0x0003);    // CAM_AWB_CCM_L_5
	MT9M114_write_cmos_sensor( 0xC89E, 0xFF9A);    // CAM_AWB_CCM_L_6
	MT9M114_write_cmos_sensor( 0xC8A0, 0xFEB4);    // CAM_AWB_CCM_L_7
	MT9M114_write_cmos_sensor( 0xC8A2, 0x024D);    // CAM_AWB_CCM_L_8
	MT9M114_write_cmos_sensor( 0xC8A4, 0x01BF);    // CAM_AWB_CCM_M_0
	MT9M114_write_cmos_sensor( 0xC8A6, 0xFF01);    // CAM_AWB_CCM_M_1
	MT9M114_write_cmos_sensor( 0xC8A8, 0xFFF3);    // CAM_AWB_CCM_M_2
	MT9M114_write_cmos_sensor( 0xC8AA, 0xFF75);    // CAM_AWB_CCM_M_3
	MT9M114_write_cmos_sensor( 0xC8AC, 0x0198);    // CAM_AWB_CCM_M_4
	MT9M114_write_cmos_sensor( 0xC8AE, 0xFFFD);    // CAM_AWB_CCM_M_5
	MT9M114_write_cmos_sensor( 0xC8B0, 0xFF9A);    // CAM_AWB_CCM_M_6
	MT9M114_write_cmos_sensor( 0xC8B2, 0xFEE7);    // CAM_AWB_CCM_M_7
	MT9M114_write_cmos_sensor( 0xC8B4, 0x02A8);    // CAM_AWB_CCM_M_8
	MT9M114_write_cmos_sensor( 0xC8B6, 0x01D9);    // CAM_AWB_CCM_R_0
	MT9M114_write_cmos_sensor( 0xC8B8, 0xFF26);    // CAM_AWB_CCM_R_1
	MT9M114_write_cmos_sensor( 0xC8BA, 0xFFF3);    // CAM_AWB_CCM_R_2
	MT9M114_write_cmos_sensor( 0xC8BC, 0xFFB3);    // CAM_AWB_CCM_R_3
	MT9M114_write_cmos_sensor( 0xC8BE, 0x0132);    // CAM_AWB_CCM_R_4
	MT9M114_write_cmos_sensor( 0xC8C0, 0xFFE8);    // CAM_AWB_CCM_R_5
	MT9M114_write_cmos_sensor( 0xC8C2, 0xFFDA);    // CAM_AWB_CCM_R_6
	MT9M114_write_cmos_sensor( 0xC8C4, 0xFECD);    // CAM_AWB_CCM_R_7
	MT9M114_write_cmos_sensor( 0xC8C6, 0x02C2);    // CAM_AWB_CCM_R_8
	MT9M114_write_cmos_sensor( 0xC8C8, 0x0075);    // CAM_AWB_CCM_L_RG_GAIN
	MT9M114_write_cmos_sensor( 0xC8CA, 0x011C);    // CAM_AWB_CCM_L_BG_GAIN
	MT9M114_write_cmos_sensor( 0xC8CC, 0x009A);    // CAM_AWB_CCM_M_RG_GAIN
	MT9M114_write_cmos_sensor( 0xC8CE, 0x0105);    // CAM_AWB_CCM_M_BG_GAIN
	MT9M114_write_cmos_sensor( 0xC8D0, 0x00A4);    // CAM_AWB_CCM_R_RG_GAIN
	MT9M114_write_cmos_sensor( 0xC8D2, 0x00AC);    // CAM_AWB_CCM_R_BG_GAIN
	MT9M114_write_cmos_sensor( 0xC8D4, 0x0A8C);    // CAM_AWB_CCM_L_CTEMP
	MT9M114_write_cmos_sensor( 0xC8D6, 0x0F0A);    // CAM_AWB_CCM_M_CTEMP
	MT9M114_write_cmos_sensor( 0xC8D8, 0x1964);    // CAM_AWB_CCM_R_CTEMP
	MT9M114_write_cmos_sensor( 0xC914, 0x0000);    // CAM_STAT_AWB_CLIP_WINDOW_XSTART
	MT9M114_write_cmos_sensor( 0xC916, 0x0000);    // CAM_STAT_AWB_CLIP_WINDOW_YSTART
	MT9M114_write_cmos_sensor( 0xC918, 0x04FF);    // CAM_STAT_AWB_CLIP_WINDOW_XEND
	MT9M114_write_cmos_sensor( 0xC91A, 0x02CF);    // CAM_STAT_AWB_CLIP_WINDOW_YEND
	MT9M114_write_cmos_sensor( 0xC904, 0x0033);    // CAM_AWB_AWB_XSHIFT_PRE_ADJ
	MT9M114_write_cmos_sensor( 0xC906, 0x0040);    // CAM_AWB_AWB_YSHIFT_PRE_ADJ
	MT9M114_write_cmos_sensor_8( 0xC8F2, 0x03);    // CAM_AWB_AWB_XSCALE
	MT9M114_write_cmos_sensor_8( 0xC8F3, 0x02);    // CAM_AWB_AWB_YSCALE
	MT9M114_write_cmos_sensor( 0xC906, 0x003C);    // CAM_AWB_AWB_YSHIFT_PRE_ADJ
	MT9M114_write_cmos_sensor( 0xC8F4, 0x0000);    // CAM_AWB_AWB_WEIGHTS_0
	MT9M114_write_cmos_sensor( 0xC8F6, 0x0000);    // CAM_AWB_AWB_WEIGHTS_1
	MT9M114_write_cmos_sensor( 0xC8F8, 0x0000);    // CAM_AWB_AWB_WEIGHTS_2
	MT9M114_write_cmos_sensor( 0xC8FA, 0xE724);    // CAM_AWB_AWB_WEIGHTS_3
	MT9M114_write_cmos_sensor( 0xC8FC, 0x1583);    // CAM_AWB_AWB_WEIGHTS_4
	MT9M114_write_cmos_sensor( 0xC8FE, 0x2045);    // CAM_AWB_AWB_WEIGHTS_5
	MT9M114_write_cmos_sensor( 0xC900, 0x03FF);    // CAM_AWB_AWB_WEIGHTS_6
	MT9M114_write_cmos_sensor( 0xC902, 0x007C);    // CAM_AWB_AWB_WEIGHTS_7
	MT9M114_write_cmos_sensor_8( 0xC90C, 0x80);    // CAM_AWB_K_R_L
	MT9M114_write_cmos_sensor_8( 0xC90D, 0x80);    // CAM_AWB_K_G_L
	MT9M114_write_cmos_sensor_8( 0xC90E, 0x80);    // CAM_AWB_K_B_L
	MT9M114_write_cmos_sensor_8( 0xC90F, 0x88);    // CAM_AWB_K_R_R
	MT9M114_write_cmos_sensor_8( 0xC910, 0x80);    // CAM_AWB_K_G_R
	MT9M114_write_cmos_sensor_8( 0xC911, 0x80);    // CAM_AWB_K_B_R


	//[Step7-CPIPE_Preference]
	MT9M114_write_cmos_sensor( 0x098E, 0x4926);    // LOGICAL_ADDRESS_ACCESS [CAM_LL_START_BRIGHTNESS]
	MT9M114_write_cmos_sensor( 0xC926, 0x0020);    // CAM_LL_START_BRIGHTNESS
	MT9M114_write_cmos_sensor( 0xC928, 0x009A);    // CAM_LL_STOP_BRIGHTNESS
	MT9M114_write_cmos_sensor( 0xC946, 0x0070);    // CAM_LL_START_GAIN_METRIC
	MT9M114_write_cmos_sensor( 0xC948, 0x00F3);    // CAM_LL_STOP_GAIN_METRIC
	MT9M114_write_cmos_sensor( 0xC952, 0x0020);    // CAM_LL_START_TARGET_LUMA_BM
	MT9M114_write_cmos_sensor( 0xC954, 0x009A);    // CAM_LL_STOP_TARGET_LUMA_BM
	MT9M114_write_cmos_sensor_8( 0xC92A, 0x80);    // CAM_LL_START_SATURATION
	MT9M114_write_cmos_sensor_8( 0xC92B, 0x7D);    // CAM_LL_END_SATURATION
	MT9M114_write_cmos_sensor_8( 0xC92C, 0x00);    // CAM_LL_START_DESATURATION
	MT9M114_write_cmos_sensor_8( 0xC92D, 0xFF);    // CAM_LL_END_DESATURATION
	MT9M114_write_cmos_sensor_8( 0xC92E, 0x3C);    // CAM_LL_START_DEMOSAIC
	MT9M114_write_cmos_sensor_8( 0xC92F, 0x02);    // CAM_LL_START_AP_GAIN
	MT9M114_write_cmos_sensor_8( 0xC930, 0x06);    // CAM_LL_START_AP_THRESH
	MT9M114_write_cmos_sensor_8( 0xC931, 0x64);    // CAM_LL_STOP_DEMOSAIC
	MT9M114_write_cmos_sensor_8( 0xC932, 0x01);    // CAM_LL_STOP_AP_GAIN
	MT9M114_write_cmos_sensor_8( 0xC933, 0x0C);    // CAM_LL_STOP_AP_THRESH
	MT9M114_write_cmos_sensor_8( 0xC934, 0x3C);    // CAM_LL_START_NR_RED
	MT9M114_write_cmos_sensor_8( 0xC935, 0x3C);    // CAM_LL_START_NR_GREEN
	MT9M114_write_cmos_sensor_8( 0xC936, 0x3C);    // CAM_LL_START_NR_BLUE
	MT9M114_write_cmos_sensor_8( 0xC937, 0x0F);    // CAM_LL_START_NR_THRESH
	MT9M114_write_cmos_sensor_8( 0xC938, 0x64);    // CAM_LL_STOP_NR_RED
	MT9M114_write_cmos_sensor_8( 0xC939, 0x64);    // CAM_LL_STOP_NR_GREEN
	MT9M114_write_cmos_sensor_8( 0xC93A, 0x64);    // CAM_LL_STOP_NR_BLUE
	MT9M114_write_cmos_sensor_8( 0xC93B, 0x32);    // CAM_LL_STOP_NR_THRESH
	MT9M114_write_cmos_sensor( 0xC93C, 0x0020);    // CAM_LL_START_CONTRAST_BM
	MT9M114_write_cmos_sensor( 0xC93E, 0x009A);    // CAM_LL_STOP_CONTRAST_BM
	MT9M114_write_cmos_sensor( 0xC940, 0x00DC);    // CAM_LL_GAMMA
	MT9M114_write_cmos_sensor_8( 0xC942, 0x38);    // CAM_LL_START_CONTRAST_GRADIENT
	MT9M114_write_cmos_sensor_8( 0xC943, 0x30);    // CAM_LL_STOP_CONTRAST_GRADIENT
	MT9M114_write_cmos_sensor_8( 0xC944, 0x50);    // CAM_LL_START_CONTRAST_LUMA_PERCENTAGE
	MT9M114_write_cmos_sensor_8( 0xC945, 0x19);    // CAM_LL_STOP_CONTRAST_LUMA_PERCENTAGE
	MT9M114_write_cmos_sensor( 0xC94A, 0x0230);    // CAM_LL_START_FADE_TO_BLACK_LUMA
	MT9M114_write_cmos_sensor( 0xC94C, 0x0380);    // CAM_LL_STOP_FADE_TO_BLACK_LUMA
	MT9M114_write_cmos_sensor( 0xC94E, 0x01CD);    // CAM_LL_CLUSTER_DC_TH_BM
	MT9M114_write_cmos_sensor_8( 0xC950, 0x05);    // CAM_LL_CLUSTER_DC_GATE_PERCENTAGE
	MT9M114_write_cmos_sensor_8( 0xC951, 0x40);    // CAM_LL_SUMMING_SENSITIVITY_FACTOR
	MT9M114_write_cmos_sensor_8( 0xC87B, 0x1B);    // CAM_AET_TARGET_AVERAGE_LUMA_DARK
	MT9M114_write_cmos_sensor_8( 0xC878, 0x0c);    // CAM_AET_AEMODE
	MT9M114_write_cmos_sensor( 0xC890, 0x0080);    // CAM_AET_TARGET_GAIN
	MT9M114_write_cmos_sensor( 0xC886, 0x01C8);    // CAM_AET_AE_MAX_VIRT_AGAIN
	MT9M114_write_cmos_sensor( 0xC87C, 0x005A);    // CAM_AET_BLACK_CLIPPING_TARGET
	MT9M114_write_cmos_sensor_8( 0xB42A, 0x05);    // CCM_DELTA_GAIN
	MT9M114_write_cmos_sensor_8( 0xA80A, 0x20);    // AE_TRACK_AE_TRACKING_DAMPENING_SPEED

	//[Step8-Features]
	MT9M114_write_cmos_sensor( 0x098E, 0x0000);    // LOGICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor( 0xC984, 0x8008);    // CAM_PORT_OUTPUT_CONTROL
	
	mDELAY(50);

	//[Anti-Flicker for MT9M114][50Hz]
	MT9M114_write_cmos_sensor(0x098E, 0xC88B);  // LOGICAL_ADDRESS_ACCESS [CAM_AET_FLICKER_FREQ_HZ]
	MT9M114_write_cmos_sensor_8(0xC88B, 0x32 );   // CAM_AET_FLICKER_FREQ_HZ

	// Saturation
	MT9M114_write_cmos_sensor_8(0xC92A,0x84);
	MT9M114_write_cmos_sensor_8(0xC92B,0x7D);
	 
	// AE
	//Reg = 0xC87A,0x48
	MT9M114_write_cmos_sensor_8(0xC87A,0x3C);
	 
	// Sharpness
	MT9M114_write_cmos_sensor(0x098E,0xC92F);
	MT9M114_write_cmos_sensor_8(0xC92F,0x01);
	MT9M114_write_cmos_sensor_8(0xC932,0x00);
	 
	// Target Gain
	MT9M114_write_cmos_sensor(0x098E,0x4890);
	MT9M114_write_cmos_sensor(0xC890,0x0040);

	MT9M114_write_cmos_sensor(0x337E,0x1000);

	 //[fixed 30 FPS]
	//MT9M114_write_cmos_sensor( 0xC812, 0x03EE);    // CAM_SENSOR_CFG_FRAME_LENGTH_LINES
	//MT9M114_write_cmos_sensor( 0xC88C, 0x1E02);    // CAM_AET_MAX_FRAME_RATE
	//MT9M114_write_cmos_sensor( 0xC88E, 0x1E02);    // CAM_AET_MIN_FRAME_RATE
	// change-config
	//MT9M114_write_cmos_sensor_8( 0xDC00, 0x28);    // SYSMGR_NEXT_STATE
	//MT9M114_write_cmos_sensor( 0x0080, 0x8002);    // COMMAND_REGISTER
	//mDELAY(150);
	 //Reg = 0xc940,0x00ff // CAM_LL_GAMMA


	//MT9M114_write_cmos_sensor( 0xC86C, 0x0C12);    // CAM_OUTPUT_FORMAT   // YCBCR ordering 

    MT9M114_write_cmos_sensor( 0x098E, 0x4812);      // LOGICAL_ADDRESS_ACCESS
	MT9M114_write_cmos_sensor( 0xC812, 0x03EE);    // CAM_SENSOR_CFG_FRAME_LENGTH_LINES
	MT9M114_write_cmos_sensor( 0xC88C, 0x1E02);    // CAM_AET_MAX_FRAME_RATE//0x1e02
	MT9M114_write_cmos_sensor( 0xC88E, 0x1E02);    // CAM_AET_MIN_FRAME_RATE//0x1e02
	 
	//[Change-Config]
	MT9M114_write_cmos_sensor( 0x001E, 0x0777);    // PAD_SLEW   orderring
    MT9M114_change_config_command();



	//MT9M114_write_cmos_sensor( 0x098E, 0xDC00 );    // LOGICAL_ADDRESS_ACCESS [SYSMGR_NEXT_STATE]
	//MT9M114_write_cmos_sensor_8( 0xDC00, 0x28);    // SYSMGR_NEXT_STATE
	//MT9M114_write_cmos_sensor( 0x0080, 0x8002);    // COMMAND_REGISTER
	//  POLL  COMMAND_REGISTER::HOST_COMMAND_1 =>  0x00
	////DELAY=100
	//mDELAY(150);
}
 
void MT9M114_Set_Mirror_Flip(kal_uint8 image_mirror)
{
    kal_uint16 reg_value, new_value, mirror;

    spin_lock(&mt9m114yuv_drv_lock);
    mirror = MT9M114_para.iMirror;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (mirror == image_mirror)
    {
        SENSORDB("[%s]: , same Mirror Flip setting[%d], Don't set \n", __FUNCTION__, image_mirror);
        return KAL_TRUE;
    }

    SENSORDB("[Enter]: %s, image_mirror = %d \n", __FUNCTION__, image_mirror);

    reg_value = MT9M114_read_cmos_sensor(0xC834);
    
    switch (image_mirror)
    {
        case IMAGE_NORMAL:
            new_value = (reg_value && 0xFFFC) | 0x0000;
            break;
        case IMAGE_H_MIRROR:
            new_value = (reg_value && 0xFFFC) | 0x0001;
            break;
        case IMAGE_V_MIRROR:
            new_value = (reg_value && 0xFFFC) | 0x0002;
            break;
        case IMAGE_HV_MIRROR:
            new_value = (reg_value && 0xFFFC) | 0x0003;
            break;
        default:
            // normal
            new_value = (reg_value && 0xFFFC) | 0x0000;
            break;
    }

    MT9M114_write_cmos_sensor(0xC834, new_value);

    //change config
    //MT9M114_write_cmos_sensor(0x098E, 0xDC00);		// LOGICAL_ADDRESS_ACCESS
    //MT9M114_write_cmos_sensor_8(0xDC00, 0x28);		// sysmgr_next_state
    //MT9M114_write_cmos_sensor(0x0080, 0x8002);		// COMMAND_REGISTER
    MT9M114_change_config_command();

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iMirror = image_mirror;
    spin_unlock(&mt9m114yuv_drv_lock);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}


/*************************************************************************
* FUNCTION
*   MT9M114Preview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                    MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.sensorMode = SENSOR_MODE_PREVIEW;
    spin_unlock(&mt9m114yuv_drv_lock);

    // set Mirror & Flip
    //sensor_config_data->SensorImageMirror
    MT9M114_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);		
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}

/*************************************************************************
* FUNCTION
*   MT9M114Capture
*
* DESCRIPTION
*   This function start the sensor capture mode.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                             MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    spin_lock(&mt9m114yuv_drv_lock);
    if(MT9M114_para.sensorMode == SENSOR_MODE_CAPTURE)
    {
        spin_unlock(&mt9m114yuv_drv_lock);
        SENSORDB("Entering in burstshot mode!");
        return KAL_TRUE;
    }
    MT9M114_para.sensorMode = SENSOR_MODE_CAPTURE;
    spin_unlock(&mt9m114yuv_drv_lock);

    // set Mirror & Flip
    MT9M114_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);
    

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}

/*************************************************************************
* FUNCTION
*   MT9M114_night_mode
*
* DESCRIPTION
*   This function change the sensor mode.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void MT9M114NightMode(kal_bool enable)
{
    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    if (enable) 
    {
        SENSORDB("[%s]: enable night mode \n", __FUNCTION__);
        MT9M114_write_cmos_sensor( 0x098E, 0x4812);      // LOGICAL_ADDRESS_ACCESS
		MT9M114_write_cmos_sensor( 0xC812, 0x0676);    // CAM_SENSOR_CFG_FRAME_LENGTH_LINES
		MT9M114_write_cmos_sensor( 0xC88C, 0x1200);    // CAM_AET_MAX_FRAME_RATE// 18fps
		MT9M114_write_cmos_sensor( 0xC88E, 0x1200);    // CAM_AET_MIN_FRAME_RATE//0x1e02
    } 
	else 
    {
        SENSORDB("[%s]: disable night mode \n", __FUNCTION__);
        MT9M114_write_cmos_sensor( 0x098E, 0x4812);      // LOGICAL_ADDRESS_ACCESS
		MT9M114_write_cmos_sensor( 0xC812, 0x03EE);    // CAM_SENSOR_CFG_FRAME_LENGTH_LINES
		MT9M114_write_cmos_sensor( 0xC88C, 0x1E02);    // CAM_AET_MAX_FRAME_RATE//0x1e02
		MT9M114_write_cmos_sensor( 0xC88E, 0x1E02);    // CAM_AET_MIN_FRAME_RATE//0x1e02
    }
	//change config
	MT9M114_change_config_command();

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}



/*************************************************************************
* FUNCTION
*   mt9m114yuvSetVideoMode
*
* DESCRIPTION
*   This function set the sensor video mode.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 mt9m114yuvSetVideoMode(UINT16 u2FrameRate)
{
#if 0 
	kal_uint16 framerate;

    SENSORDB("[Enter]: %s \n", __FUNCTION__);
    spin_lock(&mt9m114yuv_drv_lock);
    framerate = MT9M114_para.iFrameRate;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (framerate == u2FrameRate)
    {
        SENSORDB("[%s]: , same framerate setting[%d], Don't set \n", __FUNCTION__, u2FrameRate);
        return KAL_TRUE;
    }

	if (u2FrameRate == 30)
    {
    	SENSORDB("[%s]: FrameRate: %d fps \n", __FUNCTION__, u2FrameRate);

        MT9M114_write_cmos_sensor(0x098E, 0x4812);
        MT9M114_write_cmos_sensor(0xC812, 0x03EE);
        MT9M114_write_cmos_sensor(0xC88C, 0x1E02);
        MT9M114_write_cmos_sensor(0xC88E, 0x1E02);
    }
    else if (u2FrameRate == 15)       
    {
		SENSORDB("[%s]: FrameRate: %d fps \n", __FUNCTION__, u2FrameRate);

        MT9M114_write_cmos_sensor(0x098E, 0x4812);
        MT9M114_write_cmos_sensor(0xC812, 0x07CB);
        MT9M114_write_cmos_sensor(0xC88C, 0x0F00);
        MT9M114_write_cmos_sensor(0xC88E, 0x0F00);
    }
    else 
    {
        printk("[%s]: Wrong frame rate setting \n", __FUNCTION__);
        return KAL_FALSE;
    }   
    
    //change config
    MT9M114_change_config_command();
    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iFrameRate = u2FrameRate;
    spin_unlock(&mt9m114yuv_drv_lock);
#endif	
    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*   MT9M114_set_param_scene_mode
*
* DESCRIPTION
*   scene mode setting.
*
* PARAMETERS
*  
* 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_scene_mode(UINT16 para)
{
    SENSORDB("[Enter]: %s, scene mode = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case SCENE_MODE_OFF:
            MT9M114NightMode(FALSE);
            break;
        case SCENE_MODE_NIGHTSCENE:
            MT9M114NightMode(TRUE);
            break;
        default:
            SENSORDB("[Error]: %s, not support scene mode = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iNightMode = para;
    spin_unlock(&mt9m114yuv_drv_lock);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return KAL_TRUE;
}


/*************************************************************************
* FUNCTION
*	MT9M114_set_param_wb
*
* DESCRIPTION
*	wb setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_wb(UINT16 para)
{
    kal_uint16 wb;

    spin_lock(&mt9m114yuv_drv_lock);
    wb = MT9M114_para.iWB;
    spin_unlock(&mt9m114yuv_drv_lock);
    if (wb == para)
    {
        SENSORDB("[%s]: , same wb setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }

    SENSORDB("[Enter]: %s, wb value = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case AWB_MODE_AUTO:
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x02);   			
			mDELAY(50);
			MT9M114_write_cmos_sensor(0xAC04, 0x0288); 
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT:
            // cloudy 3300
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x00);      // UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL        
            mDELAY(50);
            MT9M114_write_cmos_sensor(0xC8F0, 0x1570);      // UVC_WHITE_BALANCE_TEMPERATURE_CONTROL  
			MT9M114_write_cmos_sensor(0xAC04, 0x0208); 

			MT9M114_write_cmos_sensor(0xAC12, 0x00AA); 
			MT9M114_write_cmos_sensor(0xAC14, 0x0097); 
            break;
        case AWB_MODE_DAYLIGHT:
            // D65  6500
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x00);      // UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL        
            mDELAY(50);
            MT9M114_write_cmos_sensor(0xC8F0, 0x1964);      // UVC_WHITE_BALANCE_TEMPERATURE_CONTROL
			MT9M114_write_cmos_sensor(0xAC04, 0x0208); 

			MT9M114_write_cmos_sensor(0xAC12, 0x00C0); 
			MT9M114_write_cmos_sensor(0xAC14, 0x0098);       
            break;
        case AWB_MODE_INCANDESCENT:
            // Alight 2856
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x00);      // UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL        
            mDELAY(50);
            MT9M114_write_cmos_sensor(0xC8F0, 0x0CE4);      // UVC_WHITE_BALANCE_TEMPERATURE_CONTROL
			MT9M114_write_cmos_sensor(0xAC04, 0x0208); 

			MT9M114_write_cmos_sensor(0xAC12, 0x00AA); 
			MT9M114_write_cmos_sensor(0xAC14, 0x0107);         
            break;
        case AWB_MODE_FLUORESCENT:
            // CWF 3850
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x00);      // UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL        
            mDELAY(50);
            MT9M114_write_cmos_sensor(0xC8F0, 0x0A8C);      // UVC_WHITE_BALANCE_TEMPERATURE_CONTROL
			MT9M114_write_cmos_sensor(0xAC04, 0x0208); 

			MT9M114_write_cmos_sensor(0xAC12, 0x0083); 
			MT9M114_write_cmos_sensor(0xAC14, 0x014C);

            break;
        case AWB_MODE_TUNGSTEN:
            // Alight 2856
            MT9M114_write_cmos_sensor(0x098E, 0x0000);
            MT9M114_write_cmos_sensor_8(0xC909, 0x00);      // UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL        
            mDELAY(50);
            MT9M114_write_cmos_sensor(0xC8F0, 0x0B47);      // UVC_WHITE_BALANCE_TEMPERATURE_CONTROL
			MT9M114_write_cmos_sensor(0xAC04, 0x0208); 

			MT9M114_write_cmos_sensor(0xAC12, 0x0083); 
			MT9M114_write_cmos_sensor(0xAC14, 0x011C);

            break;
        default:
            SENSORDB("[Error]: %s, not support wb value = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }


    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iWB = para;
    spin_unlock(&mt9m114yuv_drv_lock);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return KAL_TRUE;
}


/*************************************************************************
* FUNCTION
*	MT9M114_set_param_effect
*
* DESCRIPTION
*	effect setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_effect(UINT16 para)
{
    kal_uint16 effect;

    spin_lock(&mt9m114yuv_drv_lock);
    effect = MT9M114_para.iEffect;
    spin_unlock(&mt9m114yuv_drv_lock);
    if (effect == para)
    {
        SENSORDB("[%s]: , same effect[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }

    SENSORDB("[Enter]: %s, effect = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case MEFFECT_OFF:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x00);      // CAM_SFX_CONTROL
            break;
        case MEFFECT_NEGATIVE:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x03);      // CAM_SFX_CONTROL
            break;
        case MEFFECT_SEPIA:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x02);      // CAM_SFX_CONTROL
            MT9M114_write_cmos_sensor_8(0xC876, 0x1B);      // CAM_SFX_SEPIA_CR
            MT9M114_write_cmos_sensor_8(0xC877, 0xF2);      // CAM_SFX_SEPIA_CB
            break;
        case MEFFECT_SEPIAGREEN:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x02);      // CAM_SFX_CONTROL
            MT9M114_write_cmos_sensor_8(0xC876, 0xEC);      // CAM_SFX_SEPIA_CR
            MT9M114_write_cmos_sensor_8(0xC877, 0xEC);      // CAM_SFX_SEPIA_CB
            break;
        case MEFFECT_SEPIABLUE:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x02);      // CAM_SFX_CONTROL
            MT9M114_write_cmos_sensor_8(0xC876, 0xBB);      // CAM_SFX_SEPIA_CR
            MT9M114_write_cmos_sensor_8(0xC877, 0x2B);      // CAM_SFX_SEPIA_CB
            break;
        case MEFFECT_MONO:
            MT9M114_write_cmos_sensor(0x098E, 0xC874);      // LOGICAL_ADDRESS_ACCESS [CAM_SFX_CONTROL]
            MT9M114_write_cmos_sensor_8(0xC874, 0x01);      // CAM_SFX_CONTROL
            break;

        default:
            SENSORDB("[Error] %s, not support effect = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }

    // refresh command
    MT9M114_write_cmos_sensor(0x0080, 0x8004);		// COMMAND_REGISTER

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iEffect = para;
    spin_unlock(&mt9m114yuv_drv_lock);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);

	return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*	MT9M114_set_param_exposure
*
* DESCRIPTION
*	exposure setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_exposure(UINT16 para)
{
    kal_uint16 base_target = 0, exposureValue;

    spin_lock(&mt9m114yuv_drv_lock);
    exposureValue = MT9M114_para.iEV;
    spin_unlock(&mt9m114yuv_drv_lock);
    if (exposureValue == para)
    {
        SENSORDB("[%s]: , same exposure[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }

    SENSORDB("[Enter]: %s, exposure level = %d \n", __FUNCTION__, para);
    switch (para)
    {
        case AE_EV_COMP_20:
            MT9M114_write_cmos_sensor(0x098E, 0xC87A);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_TARGET_AVERAGE_LUMA]
            MT9M114_write_cmos_sensor_8(0xC87A, 0x77);      // CAM_AET_TARGET_AVERAGE_LUMA
            break;
        case AE_EV_COMP_10:
            MT9M114_write_cmos_sensor(0x098E, 0xC87A);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_TARGET_AVERAGE_LUMA]
            MT9M114_write_cmos_sensor_8(0xC87A, 0x67);      // CAM_AET_TARGET_AVERAGE_LUMA
            break;
        case AE_EV_COMP_00:
            MT9M114_write_cmos_sensor(0x098E, 0xC87A);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_TARGET_AVERAGE_LUMA]
            MT9M114_write_cmos_sensor_8(0xC87A, 0x3C);      // CAM_AET_TARGET_AVERAGE_LUMA//0x37
            break;
        case AE_EV_COMP_n10:
            MT9M114_write_cmos_sensor(0x098E, 0xC87A);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_TARGET_AVERAGE_LUMA]
            MT9M114_write_cmos_sensor_8(0xC87A, 0x1E);      // CAM_AET_TARGET_AVERAGE_LUMA
            break;
        case AE_EV_COMP_n20:
            MT9M114_write_cmos_sensor(0x098E, 0xC87A);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_TARGET_AVERAGE_LUMA]
            MT9M114_write_cmos_sensor_8(0xC87A, 0x0E);      // CAM_AET_TARGET_AVERAGE_LUMA
            break;

        default:
            SENSORDB("[Error] %s, not support exposure = %d \n", __FUNCTION__, para);
            return KAL_FALSE;      
    }

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iEV = para;
    spin_unlock(&mt9m114yuv_drv_lock);

	SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*	MT9M114_set_param_banding
*
* DESCRIPTION
*	banding setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_banding(UINT16 para)
{
    kal_uint16 banding;

    spin_lock(&mt9m114yuv_drv_lock);
    banding = MT9M114_para.iBanding;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (banding == para)
    {
        SENSORDB("[%s]: , same banding setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, banding para = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
            MT9M114_write_cmos_sensor(0x098E, 0xC88B);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_FLICKER_FREQ_HZ]
            MT9M114_write_cmos_sensor_8(0xC88A, 0x05);
            MT9M114_write_cmos_sensor_8(0xC88B, 0x32);      // CAM_AET_FLICKER_FREQ_HZ
            break;
        case AE_FLICKER_MODE_60HZ:
            MT9M114_write_cmos_sensor(0x098E, 0xC88B);      // LOGICAL_ADDRESS_ACCESS [CAM_AET_FLICKER_FREQ_HZ]
            MT9M114_write_cmos_sensor_8(0xC88A, 0x05);
            MT9M114_write_cmos_sensor_8(0xC88B, 0x3C);      // CAM_AET_FLICKER_FREQ_HZ
            break;
        default:
            SENSORDB("[Error] %s, not support banding setting = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }

    //change config
    MT9M114_change_config_command();

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iBanding = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
    return KAL_TRUE;
    
}
#if 1 //add by yujianhua
/*************************************************************************
* FUNCTION
*	MT9M114_set_param_iso
*
* DESCRIPTION
*	iso setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_iso(UINT16 para)
{
    kal_uint16 iso;

    spin_lock(&mt9m114yuv_drv_lock);
    iso = MT9M114_para.iIso;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (iso == para)
    {
        SENSORDB("[%s]: , same iso setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, iso para = %d \n", __FUNCTION__, para);

    switch (para)
    {
    	case AE_ISO_AUTO:
            MT9M114_write_cmos_sensor(0x098E, 0x2804);	// LOGICAL_ADDRESS_ACCESS [AE_TRACK_ALGO]
            MT9M114_write_cmos_sensor(0xA804, 0x00FF);	// AE_TRACK_ALGO	
            break;
        case AE_ISO_100:
            MT9M114_write_cmos_sensor(0x098E, 0x2804);	// LOGICAL_ADDRESS_ACCESS [AE_TRACK_ALGO]
            MT9M114_write_cmos_sensor(0xA804, 0x0000);	// AE_TRACK_ALGO	
            MT9M114_write_cmos_sensor(0x305E, 0x1020);  // GLOBAL_GAIN_REG   
            break;
        case AE_ISO_200:
            MT9M114_write_cmos_sensor(0x098E, 0x2804);	// LOGICAL_ADDRESS_ACCESS [AE_TRACK_ALGO]
            MT9M114_write_cmos_sensor(0xA804, 0x0000);	// AE_TRACK_ALGO	
            MT9M114_write_cmos_sensor(0x305E, 0x1033);  // GLOBAL_GAIN_REG   
            break;
		case AE_ISO_400:
            MT9M114_write_cmos_sensor(0x098E, 0x2804);	// LOGICAL_ADDRESS_ACCESS [AE_TRACK_ALGO]
            MT9M114_write_cmos_sensor(0xA804, 0x0000);	// AE_TRACK_ALGO	
            MT9M114_write_cmos_sensor(0x305E, 0x1072);  // GLOBAL_GAIN_REG   
            break;
		case AE_ISO_800:
            MT9M114_write_cmos_sensor(0x098E, 0x2804);	// LOGICAL_ADDRESS_ACCESS [AE_TRACK_ALGO]
            MT9M114_write_cmos_sensor(0xA804, 0x0000);	// AE_TRACK_ALGO	
            MT9M114_write_cmos_sensor(0x305E, 0x10B2);  // GLOBAL_GAIN_REG   
            break;
			
        default:
            SENSORDB("[Error] %s, not support iso setting = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }


    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iIso = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
    return KAL_TRUE;
    
}


/*************************************************************************
* FUNCTION
*	MT9M114_set_param_saturation
*
* DESCRIPTION
*	saturation setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_saturation(UINT16 para)
{
    kal_uint16 saturation;

    spin_lock(&mt9m114yuv_drv_lock);
    saturation = MT9M114_para.iSaturation;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (saturation == para)
    {
        SENSORDB("[%s]: , same saturation setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, saturation para = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case ISP_SAT_LOW:
            MT9M114_write_cmos_sensor(0x098E, 0x4C12);	// LOGICAL_ADDRESS_ACCESS [UVC_SATURATION_CONTROL]
            MT9M114_write_cmos_sensor(0xCC12, 0x0040);	// UVC_SATURATION_CONTROL
            break;
        case ISP_SAT_MIDDLE:
            MT9M114_write_cmos_sensor(0x098E, 0x4C12);	// LOGICAL_ADDRESS_ACCESS [UVC_SATURATION_CONTROL]
            MT9M114_write_cmos_sensor(0xCC12, 0x0080);	// UVC_SATURATION_CONTROL
            break;
		case ISP_SAT_HIGH:
            MT9M114_write_cmos_sensor(0x098E, 0x4C12);	// LOGICAL_ADDRESS_ACCESS [UVC_SATURATION_CONTROL]
            MT9M114_write_cmos_sensor(0xCC12, 0x00C0);	// UVC_SATURATION_CONTROL
            break;
			
        default:
            SENSORDB("[Error] %s, not support saturation setting = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }


    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iSaturation = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
    return KAL_TRUE;
    
}


/*************************************************************************
* FUNCTION
*	MT9M114_set_param_contrast
*
* DESCRIPTION
*	contrast setting.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
BOOL MT9M114_set_param_contrast(UINT16 para)
{
    kal_uint16 contrast;

    spin_lock(&mt9m114yuv_drv_lock);
    contrast = MT9M114_para.iContrast;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (contrast == para)
    {
        SENSORDB("[%s]: , same contrast setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, contrast para = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case ISP_CONTRAST_LOW:
            MT9M114_write_cmos_sensor(0x098E, 0xC942);	// LOGICAL_ADDRESS_ACCESS [CAM_LL_START_CONTRAST_GRADIENT]
            MT9M114_write_cmos_sensor_8(0xC942, 0x18);	// CAM_LL_START_CONTRAST_GRADIENT
            break;
		case ISP_CONTRAST_MIDDLE:
            MT9M114_write_cmos_sensor(0x098E, 0xC942);	// LOGICAL_ADDRESS_ACCESS [CAM_LL_START_CONTRAST_GRADIENT]
            MT9M114_write_cmos_sensor_8(0xC942, 0x38);	// CAM_LL_START_CONTRAST_GRADIENT
            break;
		case ISP_CONTRAST_HIGH:
            MT9M114_write_cmos_sensor(0x098E, 0xC942);	// LOGICAL_ADDRESS_ACCESS [CAM_LL_START_CONTRAST_GRADIENT]
            MT9M114_write_cmos_sensor_8(0xC942, 0x58);	// CAM_LL_START_CONTRAST_GRADIENT
            break;
      
        default:
            SENSORDB("[Error] %s, not support contrast setting = %d \n", __FUNCTION__, para);
            return KAL_FALSE;
    }

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iContrast = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
    return KAL_TRUE;
    
}
BOOL MT9M114_set_brightness(UINT16 para)
{
    kal_uint16 brightness;

    spin_lock(&mt9m114yuv_drv_lock);
    brightness = MT9M114_para.ibrightness;
    spin_unlock(&mt9m114yuv_drv_lock);

    if (brightness== para)
    {
        SENSORDB("[%s]: , same brightness setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, brightness para = %d \n", __FUNCTION__, para);

    switch (para)
    {
        case ISP_BRIGHT_LOW:
			MT9M114_write_cmos_sensor(0x098E, 0x4C0A);
            MT9M114_write_cmos_sensor_8(0xCC0A,0x46);
             break;
        case ISP_BRIGHT_MIDDLE:
			MT9M114_write_cmos_sensor(0x098E, 0x4C0A);
			MT9M114_write_cmos_sensor_8(0xCC0A,0x55);
			 break;
		case ISP_BRIGHT_HIGH:
			MT9M114_write_cmos_sensor(0x098E, 0x4C0A);
			MT9M114_write_cmos_sensor_8(0xCC0A,0x71);
			 break;

        default:
             return KAL_FALSE;
    }

	spin_lock(&mt9m114yuv_drv_lock);
	MT9M114_para.ibrightness = para;
	spin_unlock(&mt9m114yuv_drv_lock);

	SENSORDB("[Exit]: %s \n", __FUNCTION__);

	return KAL_TRUE;

}

BOOL MT9M114_set_hue(UINT16 para)
{
	kal_uint16 hue;

	spin_lock(&mt9m114yuv_drv_lock);
	hue=MT9M114_para.ihue;
	spin_unlock(&mt9m114yuv_drv_lock);	 

    if (hue == para)
    {
        SENSORDB("[%s]: , same hue setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, hue para = %d \n", __FUNCTION__, para);
	

	switch (para)
	{

		case ISP_HUE_LOW:
			break;
		case ISP_HUE_MIDDLE:
			break;
		case ISP_HUE_HIGH:
			break;
		default:
			 SENSORDB("[Error] %s, not support hue setting = %d \n", __FUNCTION__, para);
             return KAL_FALSE;

	}


    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.ihue = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
	return KAL_TRUE;
}

BOOL MT9M114_set_edge(UINT16 para)
{
	kal_uint16 edge;

	spin_lock(&mt9m114yuv_drv_lock);
	edge=MT9M114_para.iedge;
	spin_unlock(&mt9m114yuv_drv_lock);	 

    if (edge== para)
    {
        SENSORDB("[%s]: , same edge setting[%d], Don't set \n", __FUNCTION__, para);
        return KAL_TRUE;
    }
    SENSORDB("[Enter]: %s, edge para = %d \n", __FUNCTION__, para);
	

	switch (para)
	{
		case ISP_EDGE_HIGH:
			MT9M114_write_cmos_sensor(0x098E, 0x4C1C);	
			MT9M114_write_cmos_sensor(0xCC14, 0x0007);
			 break;
		case ISP_EDGE_MIDDLE:
			MT9M114_write_cmos_sensor(0x098E, 0x4C1C); 
			MT9M114_write_cmos_sensor(0xCC14, 0x0000);
			 break;
		case ISP_EDGE_LOW:
			MT9M114_write_cmos_sensor(0x098E, 0x4C1C); 
			MT9M114_write_cmos_sensor(0xCC14, 0xFFFE);
			 break;
		default:
			 SENSORDB("[Error] %s, not support edge setting = %d \n", __FUNCTION__, para);
             return KAL_FALSE;

	}

    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_para.iedge = para;
    spin_unlock(&mt9m114yuv_drv_lock);
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);
	return KAL_TRUE;
}

#endif
UINT32 mt9m114yuvSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    SENSORDB("[Enter]: %s,iCmd=%d \n", __FUNCTION__,iCmd);

    switch (iCmd)
    {
        case FID_SCENE_MODE:
            MT9M114_set_param_scene_mode(iPara);
            break;
        case FID_AWB_MODE:
            MT9M114_set_param_wb(iPara);
            break;
        case FID_COLOR_EFFECT:
            MT9M114_set_param_effect(iPara);
            break;
        case FID_AE_EV:
            MT9M114_set_param_exposure(iPara);
            break;
        case FID_AE_FLICKER:
            MT9M114_set_param_banding(iPara);
            break;
#if 1 //add by yujianhua
		case FID_AE_ISO:
            MT9M114_set_param_iso(iPara);
            break;
		case FID_ISP_SAT:
            MT9M114_set_param_saturation(iPara);
            break;
		case FID_ISP_CONTRAST:
            MT9M114_set_param_contrast(iPara);
            break;
		case FID_ISP_EDGE:
			MT9M114_set_edge(iPara);
			break;
		//case FID_ISP_HUE:
		//MT9M114_set_hue(iPara);
		//break;
		//case FID_ISP_BRIGHT:
		//MT9M114_set_brightness(iPara);
		//break;

#endif
        case FID_ZOOM_FACTOR:
            break;
        default:
            break;
    }
    
    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return TRUE;
}

/*************************************************************************
* FUNCTION
*    mt9m114yuvGetEvAwbRef
*
* DESCRIPTION
*    This function get sensor Ev/Awb (EV05/EV13) for auto scene detect
*
* PARAMETERS
*    Ref
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void mt9m114yuvGetEvAwbRef(UINT32 pSensorAEAWBRefStruct/*PSENSOR_AE_AWB_REF_STRUCT Ref*/)
{
    PSENSOR_AE_AWB_REF_STRUCT Ref = (PSENSOR_AE_AWB_REF_STRUCT)pSensorAEAWBRefStruct;
    
    SENSORDB("[Enter]: %s \n", __FUNCTION__);
    	
    Ref->SensorAERef.AeRefLV05Shutter = 2989;
    Ref->SensorAERef.AeRefLV05Gain = 94 * 4; /* 7.75x, 128 base */
    Ref->SensorAERef.AeRefLV13Shutter = 33;
    Ref->SensorAERef.AeRefLV13Gain = 32 * 4; /* 1x, 128 base */
    Ref->SensorAwbGainRef.AwbRefD65Rgain = 191; /* 1.46875x, 128 base */
    Ref->SensorAwbGainRef.AwbRefD65Bgain = 147; /* 1x, 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFRgain = 164; /* 1.25x, 128 base */
    Ref->SensorAwbGainRef.AwbRefCWFBgain = 241; /* 1.28125x, 128 base */

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}

/*************************************************************************
* FUNCTION
*    mt9m114yuvGetCurAeAwbInfo
*
* DESCRIPTION
*    This function get sensor cur Ae/Awb for auto scene detect
*
* PARAMETERS
*    Info
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void mt9m114yuvGetCurAeAwbInfo(UINT32 pSensorAEAWBCurStruct/*PSENSOR_AE_AWB_CUR_STRUCT Info*/)
{
    PSENSOR_AE_AWB_CUR_STRUCT Info = (PSENSOR_AE_AWB_CUR_STRUCT)pSensorAEAWBCurStruct;

    kal_uint16 Shutter, Gain, newGain, RGain, BGain;

    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    Shutter = MT9M114_read_cmos_sensor(0x3012);
    Info->SensorAECur.AeCurShutter = Shutter;

    Gain = MT9M114_read_cmos_sensor(0x305E);
    newGain = (((Gain & 0x0080) >> 7) * 3 + 1) * (((Gain & 0x0040) >> 6) * 1 + 1)* (Gain & 0x003F)*4;
    //newGain = ((Gain & 0x8000) >> 15) * 8 + ((Gain & 0x4000) >> 14) * 4 + ((Gain & 0x2000) >> 13) * 8 + ((Gain & 0x1000) >> 12) * 1;

    Info->SensorAECur.AeCurGain = newGain; /* 128 base */

    MT9M114_write_cmos_sensor(0x098E, 0xAC12);
    RGain = MT9M114_read_cmos_sensor(0xAC12);
    Info->SensorAwbGainCur.AwbCurRgain = RGain; /* 128 base */
    MT9M114_write_cmos_sensor(0x098E, 0xAC14);
    BGain = MT9M114_read_cmos_sensor(0xAC14);
    Info->SensorAwbGainCur.AwbCurBgain = BGain; /* 128 base */

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}


void MT9M114GetAFMaxNumFocusAreas(UINT32 *pFeatureReturnPara32)
{	
    SENSORDB("[Enter]: %s \n", __FUNCTION__);
    
    *pFeatureReturnPara32 = 1;
    SENSORDB("[MT9M114GetAFMaxNumFocusAreas], *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}

void MT9M114GetAFMaxNumMeteringAreas(UINT32 *pFeatureReturnPara32)
{	
    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    *pFeatureReturnPara32 = 1;    
    SENSORDB("[MT9M114GetAFMaxNumMeteringAreas], *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}

void MT9M114GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;

    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    spin_lock(&mt9m114yuv_drv_lock);
    
    pExifInfo->FNumber = 22;
    pExifInfo->AEISOSpeed = AE_ISO_100;
    pExifInfo->AWBMode = MT9M114_para.iWB;
    pExifInfo->CapExposureTime = MT9M114_para.iEV;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = AE_ISO_100;
    
    spin_unlock(&mt9m114yuv_drv_lock);

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
}


/*************************************************************************
* FUNCTION
*	MT9M114Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114Open(void)
{
    kal_uint16 sensor_id;

    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    sensor_id = MT9M114GetSensorID();
    
    SENSORDB("yulianfeng : MT9M114Open sensor_id is %x \n", sensor_id);
    
    if (sensor_id != MT9M114_SENSOR_ID)
    {
        SENSORDB("Error: read sensor ID fail\n");
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    /* Set initail para, please sync with initial setting */
    MT9M114InitialPara();
    
    /* Apply sensor initail setting */
    MT9M114InitialSetting();

    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    SENSORDB("yulianfeng : MT9M114Open sensor_id is\n");
	return ERROR_NONE;
} /* MT9M114Open() */

/*************************************************************************
* FUNCTION
*	MT9M114Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 MT9M114Close(void)
{
    
    return ERROR_NONE;
} /* MT9M114Close() */


UINT32 MT9M114GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("[Enter]: %s \n", __FUNCTION__);

    pSensorResolution->SensorPreviewWidth = MT9M114_IMAGE_SENSOR_PV_WIDTH - 8;
    pSensorResolution->SensorPreviewHeight = MT9M114_IMAGE_SENSOR_PV_HEIGHT - 6;
    pSensorResolution->SensorFullWidth = MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8;
    pSensorResolution->SensorFullHeight = MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6;
	pSensorResolution->SensorVideoWidth = MT9M114_IMAGE_SENSOR_VIDEO_WIDTH - 8;
    pSensorResolution->SensorVideoHeight = MT9M114_IMAGE_SENSOR_VIDEO_HEIGHT - 6;

    SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
	return ERROR_NONE;
} /* MT9M114GetResolution() */

UINT32 MT9M114GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[Enter]: %s, ScenarioId = %d \n", __FUNCTION__, ScenarioId);

    switch(ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            pSensorInfo->SensorPreviewResolutionX = MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8; /* not use */
            pSensorInfo->SensorPreviewResolutionY = MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 30; /* not use */

            break;

        default:
            pSensorInfo->SensorPreviewResolutionX = MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8; /* not use */
            pSensorInfo->SensorPreviewResolutionY = MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6; /* not use */
            pSensorInfo->SensorCameraPreviewFrameRate = 30; /* not use */

            break;
    }	
	pSensorInfo->SensorFullResolutionX=MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8;
	pSensorInfo->SensorFullResolutionY=MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6;
    pSensorInfo->SensorVideoFrameRate = 30; /* not use */
	pSensorInfo->SensorStillCaptureFrameRate=30; /* not use */
	pSensorInfo->SensorWebCamCaptureFrameRate=30; /* not use */

    pSensorInfo->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW; /* not use */
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
    pSensorInfo->SensorInterruptDelayLines = 1; /* not use */
    pSensorInfo->SensorResetActiveHigh = FALSE; /* not use */
    pSensorInfo->SensorResetDelayCount = 5; /* not use */

    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
    pSensorInfo->SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_YUYV; 
	
    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 4; 
    pSensorInfo->VideoDelayFrame = 2;
	
	pSensorInfo->YUVAwbDelayFrame = 2; 
    pSensorInfo->YUVEffectDelayFrame = 2;

    pSensorInfo->SensorMasterClockSwitch = 0; /* not use */
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_4MA;
    //pSensorInfo->SensorDriver3D = 0; /* not use */
    pSensorInfo->AEShutDelayFrame = 0;          /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 1;    /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 2;   


    switch (ScenarioId)
    {
	    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	    //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
	    case MSDK_SCENARIO_ID_CAMERA_ZSD:
			pSensorInfo->SensorClockFreq = 26;//24
			pSensorInfo->SensorClockDividCount = 3; /* not use */
			pSensorInfo->SensorClockRisingCount = 0;
			pSensorInfo->SensorClockFallingCount = 2; /* not use */
			pSensorInfo->SensorPixelClockCount = 3; /* not use */
			pSensorInfo->SensorDataLatchCount = 2; /* not use */
	        pSensorInfo->SensorGrabStartX = MT9M114_FULL_START_X; 
	        pSensorInfo->SensorGrabStartY = MT9M114_FULL_START_Y;
			//pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;//jin			
            //pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        //pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 4; 
	        //pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            //pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            //pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 	

	        break;
	    default:
	        pSensorInfo->SensorClockFreq = 26;//24
			pSensorInfo->SensorClockDividCount = 3; /* not use */
			pSensorInfo->SensorClockRisingCount = 0;
			pSensorInfo->SensorClockFallingCount = 2; /* not use */
			pSensorInfo->SensorPixelClockCount = 3; /* not use */
			pSensorInfo->SensorDataLatchCount = 2; /* not use */
	        pSensorInfo->SensorGrabStartX = MT9M114_PV_START_X; 
	        pSensorInfo->SensorGrabStartY = MT9M114_PV_START_Y;
			//pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;	//jin			
            //pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	        //pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 4; 
	        //pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            //pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            //pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 	

	        break;
    }

	SENSORDB("[Exit]: %s \n", __FUNCTION__);
	
	return ERROR_NONE;
}	/* MT9M114GetInfo() */


UINT32 MT9M114Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[Enter]: %s, ScenarioId = %d \n", __FUNCTION__, ScenarioId);
    
    spin_lock(&mt9m114yuv_drv_lock);
    MT9M114_CurrentScenarioId = ScenarioId;
    spin_unlock(&mt9m114yuv_drv_lock);

    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            MT9M114Preview(pImageWindow, pSensorConfigData);
            break;

        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
            MT9M114Capture(pImageWindow, pSensorConfigData);
            break;
        default:
            SENSORDB("Error ScenarioId!!\n");
            break;
    }

    SENSORDB("[Exit]: %s \n", __FUNCTION__);

    return ERROR_NONE;
}	/* MT9M114Control() */


UINT32 MT9M114FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 u2Temp = 0; 
	UINT16 *pFeatureReturnPara16 = (UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16 = (UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32 = (UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32 = (UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData = (MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData = (MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    SENSORDB("[Enter]: %s, FeatureId = %d \n", __FUNCTION__, FeatureId);

    switch (FeatureId)
    {
        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++ = MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8;
            *pFeatureReturnPara16 = MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6;
            *pFeatureParaLen = 4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
			switch(MT9M114_CurrentScenarioId)
			{
			    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			    //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
				case MSDK_SCENARIO_ID_CAMERA_ZSD:
	            	*pFeatureReturnPara16++ = MT9M114_IMAGE_SENSOR_FULL_WIDTH - 8;
	            	*pFeatureReturnPara16 = MT9M114_IMAGE_SENSOR_FULL_HEIGHT - 6;
	           		*pFeatureParaLen = 4;
				     break;
				default:
					*pFeatureReturnPara16++ = MT9M114_IMAGE_SENSOR_PV_WIDTH - 8;
	            	*pFeatureReturnPara16 = MT9M114_IMAGE_SENSOR_PV_HEIGHT - 6;
	           		*pFeatureParaLen = 4;
				     break;
			}
			break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *pFeatureReturnPara32 = 96000000;
            *pFeatureParaLen = 4;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            // for raw sensor
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            MT9M114NightMode((BOOL)*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            // for raw sensor
            break;
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            // not use
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            // not use
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            // for debug
            MT9M114_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            // for debug
            pSensorRegData->RegData = MT9M114_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
            memcpy(pSensorConfigData, &MT9M114_SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen = sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
        case SENSOR_FEATURE_GET_CCT_REGISTER:
        case SENSOR_FEATURE_SET_ENG_REGISTER:
        case SENSOR_FEATURE_GET_ENG_REGISTER:
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
        case SENSOR_FEATURE_GET_GROUP_COUNT:
        case SENSOR_FEATURE_GET_GROUP_INFO:
        case SENSOR_FEATURE_GET_ITEM_INFO:
        case SENSOR_FEATURE_SET_ITEM_INFO:
        case SENSOR_FEATURE_GET_ENG_INFO:
            // for cct use in raw sensor
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
            *pFeatureReturnPara32 = LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen = 4;
            break;
        case SENSOR_FEATURE_SET_YUV_CMD:
            mt9m114yuvSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32 + 1));
            break;
        case SENSOR_FEATURE_SET_VIDEO_MODE:
            mt9m114yuvSetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_CALIBRATION_DATA:
        case SENSOR_FEATURE_SET_SENSOR_SYNC:  
        case SENSOR_FEATURE_INITIALIZE_AF:
        case SENSOR_FEATURE_CONSTANT_AF:
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
        case SENSOR_FEATURE_GET_AF_STATUS:
        case SENSOR_FEATURE_GET_AF_INF:
        case SENSOR_FEATURE_GET_AF_MACRO:
            // not use
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
            MT9M114CheckSensorID(pFeatureReturnPara32); 
            break;
        case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
        case SENSOR_FEATURE_SET_TEST_PATTERN:
        case SENSOR_FEATURE_SET_SOFTWARE_PWDN:  
        case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
        case SENSOR_FEATURE_CANCEL_AF:
        case SENSOR_FEATURE_SET_AF_WINDOW:
            // not use
            break;        
        case SENSOR_FEATURE_GET_EV_AWB_REF:
            mt9m114yuvGetEvAwbRef(*pFeatureData32);
            break;
        case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
            mt9m114yuvGetCurAeAwbInfo(*pFeatureData32);	
            break;
        case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
            MT9M114GetAFMaxNumFocusAreas(pFeatureReturnPara32);                    
            *pFeatureParaLen=4;        
            break;
        case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
            MT9M114GetAFMaxNumMeteringAreas(pFeatureReturnPara32);                    
            *pFeatureParaLen=4;        
            break;    
        case SENSOR_FEATURE_SET_AE_WINDOW:
            // not use
            break;
        case SENSOR_FEATURE_GET_EXIF_INFO:                 
            MT9M114GetExifInfo(*pFeatureData32);        
            break; 
        default:
            break;
    }

    //SENSORDB("[Exit]: %s \n", __FUNCTION__);
    
	return ERROR_NONE;
}	/* MT9M114FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncMT9M114=
{
	MT9M114Open,
	MT9M114GetInfo,
	MT9M114GetResolution,
	MT9M114FeatureControl,
	MT9M114Control,
	MT9M114Close
};

UINT32 MT9M114_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT * pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncMT9M114;

	return ERROR_NONE;
}	/* SensorInit() */


