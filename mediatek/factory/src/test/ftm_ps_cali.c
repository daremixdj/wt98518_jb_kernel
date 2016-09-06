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


#include "CFG_PS_File.h"
//#include "CFG_PS_Default.h"  //deleted by fangliang for build
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
//#include "./libfile_op.h"
#include "../../../external/nvram/libfile_op/libfile_op.h"// added by fangliang

#ifdef CUSTOM_KERNEL_ALSPS 
#include <linux/sensors_io.h>
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[PS_CALI] "
#define mod_to_ps_cali_data(p) (struct ps_cali_data*)((char*)(p) + sizeof(struct ftm_module))
#define PLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define PSLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/

//bool FileOp_BackupToBinRegion_All();// added by fangliang
#define COUNT 20
#define WT_PS_CALI_CLEAR_NOT_SUPPORT
enum {
    ITEM_CLEAR,
    ITEM_CALIBRATION,
    ITEM_EXIT,
};

enum{
    PS_OP_NONE,
    PS_OP_CLEAR,
    PS_OP_CALI_PRE,
    PS_OP_CALI_CALI,
}; 

enum{
    CLOSE,
    FAR_AWAY,
}; 



/*---------------------------------------------------------------------------*/
static item_t ps_cali_items[] = {
    item(ITEM_CALIBRATION,   uistr_info_sensor_cali_start),
#ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT
    item(ITEM_CLEAR,   uistr_info_sensor_cali_clear),
#endif
    item(ITEM_EXIT,   uistr_info_sensor_back),
    item(-1, NULL),
};

//zhaoqf_sh add for full test
static item_t ps_cali_auto_items[] = {
#ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT
    item(ITEM_CALIBRATION,   uistr_info_sensor_cali_start),
	item(ITEM_CLEAR,   uistr_info_sensor_cali_clear),
#endif    
    item(-1, NULL),
};

typedef struct
{
    int close;
    int far_away;
    int valid;
} PS_CALI_STRUCT;


/*---------------------------------------------------------------------------*/
struct ps_cali_priv
{
    /*specific data field*/
	pthread_mutex_t evtmutex;
    char    *dev;
    int     fd;
    unsigned int ps_raw;
     int  pending_op;
	int  cali_item;
	unsigned int avg;
	char status[1024];
	PS_CALI_STRUCT  pdata;
};
/*---------------------------------------------------------------------------*/
struct ps_cali_data
{
    struct ps_cali_priv ps_cali;

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
static int ps_cali_init_priv(struct ps_cali_priv *ps_cali)
{
    memset(ps_cali, 0x00, sizeof(*ps_cali));
    ps_cali->fd = -1;
    ps_cali->dev = "/dev/als_ps";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int ps_cali_open(struct ps_cali_priv *ps_cali)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    if (ps_cali->fd == -1) {
        ps_cali->fd = open("/dev/als_ps", O_RDONLY);
        if (ps_cali->fd < 0) {
            LOGD("Couldn't open '%s' (%s)", ps_cali->dev, strerror(errno));
            return -1;
        }
        retry = 0;
        while ((err = ioctl(ps_cali->fd, ALSPS_SET_PS_MODE, &flags)) && (retry ++ < max_retry))
            usleep(retry_period*1000);
        if (err) {
            LOGD("enable ps fail: %s", strerror(errno));
            return -1;            
        } 
        retry = 0;
    }
    LOGD("%s() %d\n", __func__, ps_cali->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int ps_cali_close(struct ps_cali_priv *ps_cali)
{
    unsigned int flags = 0;
    int err;
    if (ps_cali->fd != -1) {
        if ((err = ioctl(ps_cali->fd, ALSPS_SET_PS_MODE, &flags))) {
            LOGD("disable ps fail: %s", strerror(errno));
            return -1;            
        } 
        close(ps_cali->fd);
    }
    memset(ps_cali, 0x00, sizeof(*ps_cali));
    ps_cali->fd = -1;
    ps_cali->dev = "/dev/als_ps";
    return 0;
}

static int ps_cali_update_info(struct ps_cali_priv *ps_cali)
{
	int err = -EINVAL,num = 0;
	 int  ps_dat;

	// LOGD(TAG "%s: Enter\n", __FUNCTION__);    
	 
	 if (ps_cali->fd == -1) {
		 LOGD("invalid fd\n");
		//  LOGD(TAG "%s: Enter\n", __FUNCTION__);    
		 return -EINVAL;
	 } else{
		// LOGD(TAG "%s: Enter 1\n", __FUNCTION__);    

			ps_cali->ps_raw = 0;
			if(err = ioctl(ps_cali->fd, ALSPS_GET_PS_RAW_DATA, &ps_dat)) {
				 LOGD("read ps  raw: %d(%s)\n", err, strerror(errno));
				  LOGD(TAG "%s: Enter 2\n", __FUNCTION__);    
				 return err;
				 }
			 ps_cali->ps_raw = ps_dat;
			// LOGD(TAG "%s: Enter 3\n", __FUNCTION__);    

			 return 0;
	}


}


/*---------------------------------------------------------------------------*/

static int ps_calibration(struct ps_cali_priv *ps_cali)
{
	int err = -EINVAL,num = 0;
	 int  ps_dat;
	 if (ps_cali->fd == -1) {
		 LOGD("invalid fd\n");
		 return -EINVAL;
	 } else{

		 ps_cali->avg = 0;

			if(err = ioctl(ps_cali->fd, ALSPS_GET_PS_RAW_DATA_FOR_CALI, &ps_cali->pdata)) {
				 LOGD("read ps  raw: %d(%s)\n", err, strerror(errno));
				 return err;
				 }
	 	}
	 LOGD("ps_calibration: %d,%d,%d\n",ps_cali->pdata.close,ps_cali->pdata.far_away,ps_cali->pdata.valid); 
	 printf("fangliang ps_calibration: %d,%d,%d\n",ps_cali->pdata.close,ps_cali->pdata.far_away,ps_cali->pdata.valid); 
	 return 0;

}

static int ps_write_nvram(struct ps_cali_priv *ps_cali)
{	

	int file_lid = AP_CFG_RDCL_HWMON_PS_LID;    
	int rec_size;
	int rec_num;
	//int res, fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);//deleted by fangliang for build
	int res;
	F_ID fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	NVRAM_HWMON_PS_STRUCT hwmonPs =
		{	 
			{0,  0, 0},
		};	  
		
		if(fd.iFileDesc < 0)//fd.iFileDesc added by fangliang for build  [fd before]
		{
			LOGD("nvram open = %d\n", fd.iFileDesc);
			return fd.iFileDesc;
		}

		hwmonPs.close= ps_cali->pdata.close;
		hwmonPs.far_away= ps_cali->pdata.far_away;
		hwmonPs.valid= ps_cali->pdata.valid;
		
	 res = write(fd.iFileDesc, &hwmonPs , rec_size*rec_num); 
	 
	 LOGD("the nvram data_ps close = %d,far_away = %d\n",hwmonPs.close,hwmonPs.far_away);
	 printf("fangliang the nvram data_ps close = %d,far_away = %d,valid = %d \n",hwmonPs.close,hwmonPs.far_away,hwmonPs.valid);
		 if(res < 0)
		 {
			 LOGD("nvram write = %d(%s)\n", errno, strerror(errno));
			 printf("fangliang error %s\n",__func__);
			 return res;
		 }
		 NVM_CloseFileDesc(fd); 
		 FileOp_BackupToBinRegion_All();  //backup TO BinRegion  Api 
		 sync();

//lidechun  add
		property_set("ctl.start", "nvram_proximity");
//end
		 return 0;
}

static void *ps_cali_update_iv_thread(void *priv)
{
    struct ps_cali_data *dat = (struct ps_cali_data *)priv; 
    struct ps_cali_priv *lps = &dat->ps_cali;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;
    static int op = -1;
	int alsps_count = 0,alsps_ps_tmp = 0,ps_avg_data = 0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = ps_cali_open(lps))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
        iv->redraw(iv);
        LOGD("alsps() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {

  		 if(dat->exit_thd)
		   {
			   break;
		   }
 
		   pthread_mutex_lock(&dat->ps_cali.evtmutex);
		   if(op != dat->ps_cali.pending_op)
		   {
			   op = dat->ps_cali.pending_op;
			   LOGD("op: %d\n", dat->ps_cali.pending_op);
		   }
		   pthread_mutex_unlock(&dat->ps_cali.evtmutex);
		   err = 0;
  
		   if(op == PS_OP_CLEAR)
		   {
		   	memset(&dat->ps_cali.pdata, 0x00, sizeof(dat->ps_cali.pdata));
			if(err = ps_write_nvram(lps))
				{

					LOGD("ps_write_nvram: %d\n", err); 
			}
			if(err)
				{
					len = snprintf(dat->ps_cali.status, sizeof(dat->ps_cali.status), "Clear: Fail\n");  
				}
				else
				{
					len = snprintf(dat->ps_cali.status, sizeof(dat->ps_cali.status), "Clear: Pass\n");
				}

			   pthread_mutex_lock(&dat->ps_cali.evtmutex);
			   dat->ps_cali.pending_op = PS_OP_NONE;
			   pthread_mutex_unlock(&dat->ps_cali.evtmutex);
		   }
		   else if(op == PS_OP_CALI_PRE)
		   {
			   err = 0;
			   /*by-pass*/
			   snprintf(dat->ps_cali.status, sizeof(dat->ps_cali.status), "Calibrating: don't touch\n");	
			   pthread_mutex_lock(&dat->ps_cali.evtmutex);
			   dat->ps_cali.pending_op = PS_OP_CALI_CALI;
			   pthread_mutex_unlock(&dat->ps_cali.evtmutex);
		   }
		   else if(op == PS_OP_CALI_CALI)
		   {

            if(err = ps_calibration(lps))
            {
                LOGD("calibrate ps: %d\n", err); 
            }
            else if(err = ps_write_nvram(lps))
            {
                LOGD("ps_write_nvram: %d\n", err); 
            }

            if(err)
            {
                len = snprintf(dat->ps_cali.status, sizeof(dat->ps_cali.status), "Calibrating: Fail\n");
                dat->mod->test_result = FTM_TEST_FAIL;//zhaoqf_sh for test result
            }
            else
            {
                len = snprintf(dat->ps_cali.status, sizeof(dat->ps_cali.status), "Calibrating: Pass\n");
                dat->mod->test_result = FTM_TEST_PASS;//zhaoqf_sh for test result
            }
						
            pthread_mutex_lock(&dat->ps_cali.evtmutex);
            dat->ps_cali.pending_op = PS_OP_NONE;
            pthread_mutex_unlock(&dat->ps_cali.evtmutex);

		   }  
        if(err = ps_cali_update_info(lps))
        {
            LOGD("ps_cali_update_info() = (%s), %d\n", strerror(errno), err);
            break;
        } 
	//------------- ps data avg ---------
		if(alsps_count == 0){
			ps_avg_data = lps->ps_raw;
		}

		if(alsps_count < 31){
				alsps_count++;
				alsps_ps_tmp += lps->ps_raw;
			}

		if(alsps_count >= 31){
			ps_avg_data = alsps_ps_tmp/30;
			alsps_count = 1;
			alsps_ps_tmp = 0;
		}
	//------------- ps data avg ---------
        len = 0;
#if 1//zhaoqf_sh modify for ps_cali hint info display
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "PS : %d (0:far ; +:close)\n%s\n%s\n",ps_avg_data, dat->ps_cali.status, uistr_info_sensor_ps_cali_hint);
#else
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "PS : %4Xh (0:far ; +:close)\n",dat->ps_cali.ps_raw);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s",dat->ps_cali.status);
#endif
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
   
    }
    ps_cali_close(lps);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int ps_cali_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct ps_cali_data *dat = (struct ps_cali_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err,op;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, "Fail", COLOR_YELLOW);
    init_text(&dat->center_btn, "Pass", COLOR_YELLOW);
    init_text(&dat->right_btn, "Back", COLOR_YELLOW);
       
    snprintf(dat->info, sizeof(dat->info), "Initializing...\n");
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
    if(1 == is_full_mode)//zhaoqf_sh for full test
    {
    #ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT
    auto_select = true;
	#endif
    iv->set_items(iv, ps_cali_auto_items, 0);
    }
    else
    {
    iv->set_items(iv, ps_cali_items, 0);
    }
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, ps_cali_update_iv_thread, priv);

    if(1 == is_full_mode)//zhaoqf_sh add for full test 
    {
    #ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT
        pthread_mutex_lock(&dat->ps_cali.evtmutex);
        dat->ps_cali.pending_op = PS_OP_CLEAR;
        pthread_mutex_unlock(&dat->ps_cali.evtmutex);
	#else
		pthread_mutex_lock(&dat->ps_cali.evtmutex); 		   
		dat->ps_cali.pending_op= PS_OP_CALI_PRE;   
		dat->ps_cali.cali_item = CLOSE;
		pthread_mutex_unlock(&dat->ps_cali.evtmutex);
	#endif
    }
    
     do {
        chosen = iv->run(iv, &exit);    
        pthread_mutex_lock(&dat->ps_cali.evtmutex);
        op = dat->ps_cali.pending_op;
        pthread_mutex_unlock(&dat->ps_cali.evtmutex);    
        if ((chosen != ITEM_EXIT) && (op != PS_OP_NONE) )/*some OP is pending*/
            continue;
        switch (chosen) {
		#ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT			
        case ITEM_CLEAR:
            pthread_mutex_lock(&dat->ps_cali.evtmutex);
            dat->ps_cali.pending_op = PS_OP_CLEAR;
            LOGD("chosen clear: %d\n",  dat->ps_cali.pending_op);
            pthread_mutex_unlock(&dat->ps_cali.evtmutex);
            break;
		#endif
        case ITEM_CALIBRATION:
            pthread_mutex_lock(&dat->ps_cali.evtmutex);            
            dat->ps_cali.pending_op= PS_OP_CALI_PRE;   
	        dat->ps_cali.cali_item = CLOSE;
            LOGD("chosen Do calibration\n");
            pthread_mutex_unlock(&dat->ps_cali.evtmutex);
            break;
        case ITEM_EXIT:
            exit = true;            
            break;

        //zhaoqf_sh add for full test
        case 100:
        case 101:
            if(100 == chosen)
            {
                dat->mod->test_result = FTM_TEST_PASS;
            }
            else if(101 == chosen)
            {
                dat->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
        }
        
        if (exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);

	#ifndef WT_PS_CALI_CLEAR_NOT_SUPPORT
    if(1 == is_full_mode)
    {
        auto_select = false;
    }
	#endif
         
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int ps_cali_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct ps_cali_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_PS_CALI, sizeof(struct ps_cali_data));
    dat  = mod_to_ps_cali_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    ps_cali_init_priv(&dat->ps_cali);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, ps_cali_entry, (void*)dat);

    return ret;
}
#endif 
