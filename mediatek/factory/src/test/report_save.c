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
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "miniui.h"
#include "report_save.h"
#include "ftm.h"

#include "Custom_NvRam_LID.h"
#include "CFG_module_file.h"
#include "libnvram.h"
#include "../../../external/nvram/libfile_op/libfile_op.h"
#include "CFG_PRODUCT_INFO_File.h"

#define LOGD ALOGD
#define BOARDTEST_SUCCESS 100
#define BOARDTEST_FAIL 0

static char* reportlistname = 0;
static char mmi_reportlistname[] = "/data/factoryreport.log";
static char pcba_reportlistname[] = "/data/pcbafactoryreport.log";
static char testsuccessfilename[] = "/data/testsuccess";
static char pcbatestsuccessfilename[] = "/data/pcbatestsuccess";
extern int sIsBoardTest;
extern bool g_pcba_test;
extern int s_pcba_test_success;

extern void initboardtest(void);
extern void inittestmodevalue(void);

static int judgeIsBoardTestSuccess(report_save_struct* item, int count);

void init_reportfile(char ispcba)
{
    if(ispcba == true){
        reportlistname = pcba_reportlistname;
    }else{
        reportlistname = mmi_reportlistname;
    }
}

int report_create(item_t *item, char ispcba){
	int ret, n= 0;
	item_t *item_p = item;
	int isexit = 0;
	report_save_struct save_report[ITEM_MAX_IDS] = {0};
	report_save_struct read_rst[ITEM_MAX_IDS] = {0};
	report_save_struct *save_index = save_report;

    init_reportfile(ispcba);

	//check file is existed
	if(-1 != access(reportlistname, 0)){
		isexit = 1;
		 FILE *fp1 = fopen(reportlistname, "r");
		 if(fp1 == NULL){
		 	LOGD("%s: fp1 open file fail \n",__FUNCTION__);
		 	ret = -1;
			goto END;
		 }else{
		 	LOGD("%s: fp1 open file success \n",__FUNCTION__);
		 }
		fseek(fp1, 0L, SEEK_END);
		int count = ftell(fp1)/sizeof(report_save_struct);
		fseek(fp1, 0L, SEEK_SET);
	
		LOGD("%s: count = %d \n",__FUNCTION__, count);
		if(count!=0){
			fread(&read_rst , sizeof(report_save_struct), count, fp1);
		}
		fclose(fp1);
	}
	
	 FILE *fp = fopen(reportlistname, "w+");
	 if(fp == NULL){
	 	LOGD("%s: open file fail \n",__FUNCTION__);
	 	ret = -1;
		goto END;
	 }else{
	 	LOGD("%s: open file success \n",__FUNCTION__);
	 }
	 
	 while(item_p!= NULL && item_p->name!=NULL){
	 	save_index->id = item_p->id;
		LOGD("%s: save_index->id = %d \n",__FUNCTION__, save_index->id);
		LOGD("%s: save_index->value = %d \n",__FUNCTION__, getSaveValue(read_rst ,save_index->id));
		save_index->value = (isexit == 0)? 0 : getSaveValue(read_rst ,save_index->id);
		save_index ++;
	 	item_p++;
	 	n ++;
	 }
	 
	if(n==fwrite(save_report, sizeof(report_save_struct), n, fp)){
		ret = 1;
	}
	
END:
	fclose(fp);
	return ret;
}


int report_writeByID(int id, int value){	
	int ret=0;	
	int i = 0;
	int seekret = 0;
	report_save_struct read_rst[ITEM_MAX_IDS] = {0};

	FILE *fp;
    
	init_reportfile(g_pcba_test);
    
       fp = fopen(reportlistname, "r");
	if(fp == NULL){
		LOGD("%s: open file fail \n",__FUNCTION__);
		return -1;
	}else{
		LOGD("%s: open file success \n",__FUNCTION__);
	}
	
	fseek(fp, 0L, SEEK_END);
	int count = ftell(fp)/sizeof(report_save_struct);
	fseek(fp, 0L, SEEK_SET);
	
	LOGD("%s: count = %d \n",__FUNCTION__, count);
	fread(&read_rst , sizeof(report_save_struct), count, fp);
	
	for( i = 0; i<count ;i ++){	
		LOGD("%s: read_rst.id = %d , read_rst.value = %d \n",__FUNCTION__, read_rst[i].id , read_rst[i].value);
		if(read_rst[i].id == id){
			read_rst[i].value = value;
			ret = 1;			
			break;		
		}	
	}
	fclose(fp);

	FILE *fp1 = fopen(reportlistname, "w");
	if(fp1 == NULL){
		LOGD("%s: open file fail \n",__FUNCTION__);
		return -1;
	}else{
		LOGD("%s: open file success \n",__FUNCTION__);
	}	

	fwrite(&read_rst, sizeof(report_save_struct), count, fp1);
	fclose(fp1);
    
        if(sIsBoardTest != 1 && g_pcba_test == false){
            if(judgeIsBoardTestSuccess(read_rst, count) == 1){
                if(saveBoardTestSuccessFlag(1) == 1){
                    sIsBoardTest = 1;
                    initboardtest();
                    inittestmodevalue();
                }
            }
        }else if(sIsBoardTest == 1 && g_pcba_test == false){
            if(judgeIsBoardTestSuccess(read_rst, count) == 0){
                if(saveBoardTestSuccessFlag(0) == 1){
                    sIsBoardTest = 0;
                    initboardtest();
                    inittestmodevalue();
                }
            }
        }
    
        if(s_pcba_test_success != 1 && g_pcba_test == true){
            if(judgeIsBoardTestSuccess(read_rst, count) == 1){
                if(saveBoardTestSuccessFlag(1) == 1){
                    s_pcba_test_success = 1;
                    initboardtest();
                    inittestmodevalue();
                }
            }
        }else if(s_pcba_test_success == 1 && g_pcba_test == true){
            if(judgeIsBoardTestSuccess(read_rst, count) == 0){
                if(saveBoardTestSuccessFlag(0) == 1){
                    s_pcba_test_success = 0;
                    initboardtest();
                    inittestmodevalue();
                }
            }
        }
        
        sync();
	return ret;
}


int report_readByID(int id){
	
	int ret=0;
	int i = 0;
	report_save_struct read_rst[ITEM_MAX_IDS] = {0};
	FILE *fp;
    
	init_reportfile(g_pcba_test);
	fp = fopen(reportlistname, "r+");
	
	if(fp == NULL){
		LOGD("%s: open file fail \n",__FUNCTION__);
		ret = -1;
		goto END;
	}else{
		LOGD("%s: open file success \n",__FUNCTION__);
	}

	fseek(fp, 0L, SEEK_END);
	int count = ftell(fp)/sizeof(report_save_struct);
	LOGD("%s: count = %d \n",__FUNCTION__, count);
	fseek(fp, 0L, SEEK_SET);
	
	fread(&read_rst , sizeof(report_save_struct), count, fp);
	for( i = 0; i<count ;i ++){	
		LOGD("%s: read_rst.id = %d , read_rst.value = %d \n",__FUNCTION__, read_rst[i].id , read_rst[i].value);
		if(read_rst[i].id == id){
			ret = read_rst[i].value;
			break;
		}
	}
	
END:
	fclose(fp);
	LOGD("%s: fclose(fp) ok",__FUNCTION__);
	return ret;
}

int getSaveValue(report_save_struct* item, int id)
{
	report_save_struct* itemp = item;
	int ret = 0;
	int i = 0;
	while(itemp!=NULL && i <= ITEM_MAX_IDS){
		LOGD("%s: itemp->id = %d , itemp->value = %d",__FUNCTION__, itemp->id, itemp->value);
		if(itemp->id == id){
			ret = itemp->value;
			return ret;
		}
		i++;
		itemp++;
	}
	return 0;
}

int saveBoardTestSuccessFlag(int result)
{
    char buf[2] = {1, 0};    
    FILE *fp = NULL;
    int rec_size;
    int rec_num;
    int res;    
    F_ID fd;
    PRODUCT_INFO* sProductInfo;

    sProductInfo= (PRODUCT_INFO *)malloc(sizeof(PRODUCT_INFO));
    if(sProductInfo == NULL)
    {
        LOGD("malloc fail!!!!\n");
        return 0;
    }
    memset(sProductInfo, 0, sizeof(PRODUCT_INFO));
    //read from ap nv
    fd = NVM_GetFileDesc(AP_CFG_REEB_PRODUCT_INFO_LID, &rec_size, &rec_num, ISREAD);
    
    LOGD("%s: get nv param infosize = %d, size = %d, num = %d\n",__FUNCTION__, sizeof(PRODUCT_INFO), rec_size, rec_num);
    if(fd.iFileDesc < 0)
    {
        LOGD("nvram open for read= %d\n", fd.iFileDesc);
        free(sProductInfo);
        return fd.iFileDesc;
    }
    if(read(fd.iFileDesc, sProductInfo , rec_size*rec_num) < 0){
        LOGD("%s:  error!, read nv param fail\n",__FUNCTION__);
        NVM_CloseFileDesc(fd);
        free(sProductInfo);
        return 0;
    }
    NVM_CloseFileDesc(fd);

    //write ap nv
    fd = NVM_GetFileDesc(AP_CFG_REEB_PRODUCT_INFO_LID, &rec_size, &rec_num, ISWRITE);
    if(fd.iFileDesc < 0)
    {
        LOGD("nvram open for write= %d\n", fd.iFileDesc);
        free(sProductInfo);
        return fd.iFileDesc;
    }
    if(g_pcba_test == true){
        if(result == 1){
            sProductInfo->reserved[1] = BOARDTEST_SUCCESS;
        }else{
            sProductInfo->reserved[1] = BOARDTEST_FAIL;
        }
    }else{
        if(result == 1){
            sProductInfo->reserved[0] = BOARDTEST_SUCCESS;
        }else{
            sProductInfo->reserved[0] = BOARDTEST_FAIL;
        }
    }
    res = write(fd.iFileDesc, sProductInfo , rec_size*rec_num); 
    NVM_CloseFileDesc(fd); 
    if(res < 0){
        LOGD("nvram write error = %d\n", res);
        free(sProductInfo);
        return 0;
    }
    //FileOp_BackupToBinRegion_All();  //backup TO BinRegion  Api 
    //sync();
    free(sProductInfo);
    
    if(g_pcba_test == true){
        fp = fopen(pcbatestsuccessfilename, "w");    
    }else{
        fp = fopen(testsuccessfilename, "w");    
    }
    if(fp == NULL){
        LOGD("%s: open file fail \n",__FUNCTION__);
        return -1;
    }else{
        LOGD("%s: open file success \n",__FUNCTION__);
    }
    if(result == 1){
        fwrite(buf, 1, 1, fp);
    }else{
        fwrite(buf, 0, 1, fp);
    }
    fclose(fp);
    return 1;
}

static int judgeIsBoardTestSuccess(report_save_struct* item, int count)
{
    int i = 0;
    
    for(i = 0; i < count; i ++){
        LOGD("%s: i = %d, id = %d, value = %d \n",__FUNCTION__, i, 
                        item[i].id, item[i].value);
        switch(item[i].id){
		case ITEM_VERSION:
#ifdef FEATURE_FTM_KEYS
        case ITEM_KEYS:
#endif
#ifdef FEATURE_FTM_JOGBALL
        case ITEM_JOGBALL:
#endif
#ifdef FEATURE_FTM_OFN
        case ITEM_OFN:
#endif
#ifdef FEATURE_FTM_TOUCH
        case ITEM_TOUCH:
#endif    
#ifdef FEATURE_FTM_LCD
        case ITEM_LCD:
#endif
#ifdef FEATURE_FTM_LCM
	case ITEM_LCM:
#endif
#ifdef FEATURE_FTM_LED
        case ITEM_LED:
#endif
#ifdef FEATURE_FTM_MAIN_CAMERA
        case ITEM_MAIN_CAMERA:
#endif
#ifdef FEATURE_FTM_MAIN2_CAMERA
        case ITEM_MAIN2_CAMERA:
#endif
#ifdef FEATURE_FTM_SUB_CAMERA
        case ITEM_SUB_CAMERA:
#endif
#ifdef FEATURE_FTM_STROBE
		case ITEM_STROBE:
#endif
#ifndef FEATURE_FTM_WIFI_ONLY
#ifdef FEATURE_FTM_MEMCARD
        case ITEM_MEMCARD:
#endif
#ifdef FEATURE_FTM_SIMCARD
        case ITEM_SIMCARD:
#endif
#ifdef FEATURE_FTM_SIM
        case ITEM_SIM:
#endif
#endif    
#ifdef FEATURE_FTM_AUDIO
#ifdef FEATURE_FTM_RECEIVER
		case ITEM_RECEIVER:
#endif
#ifdef FEATURE_FTM_PHONE_MIC_RECEIVER_LOOPBACK
        case ITEM_LOOPBACK:
#endif
#ifdef FEATURE_FTM_ACSLB
		//case ITEM_ACOUSTICLOOPBACK:
#endif
#ifdef FEATURE_FTM_PHONE_MIC_HEADSET_LOOPBACK
        case ITEM_LOOPBACK1:
#endif
#ifdef FEATURE_FTM_PHONE_MIC_SPEAKER_LOOPBACK
#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
		case ITEM_LOOPBACK2_MIC1:
		case ITEM_LOOPBACK2_MIC2:
#else
        case ITEM_LOOPBACK2:
#endif			
#endif  
#ifdef FEATURE_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
        case ITEM_LOOPBACK3:
#endif
#ifdef FEATURE_FTM_WAVE_PLAYBACK
		case ITEM_WAVEPLAYBACK:
#endif
#endif //FEATURE_FTM_AUDIO
#ifdef FEATURE_FTM_HEADSET
        case ITEM_HEADSET:
#endif    
#ifdef FEATURE_FTM_OTG
        //case ITEM_OTG:
#endif    
#ifdef CUSTOM_KERNEL_ACCELEROMETER
        case ITEM_GSENSOR:
//        case ITEM_GS_CALI:
#endif
#ifdef CUSTOM_KERNEL_MAGNETOMETER
        case ITEM_MSENSOR:
#endif
#ifdef CUSTOM_KERNEL_ALSPS
        case ITEM_PS_CALI:
        case ITEM_ALSPS:
#endif
#ifdef CUSTOM_KERNEL_MHALL_SENSOR
        case ITEM_MHALL_SENSOR:
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
        case ITEM_BAROMETER:
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
        case ITEM_GYROSCOPE:
        //case ITEM_GYROSCOPE_CALI:
#endif
#ifdef MTK_NFC_SUPPORT
        case ITEM_NFC:
#endif
#ifdef FEATURE_FTM_TVOUT
        case ITEM_TVOUT:
#endif
#ifdef FEATURE_FTM_CMMB
        case ITEM_CMMB:
#endif
#ifdef FEATURE_FTM_EMI
        case ITEM_EMI:
#endif
#ifdef FEATURE_FTM_HDMI
        case ITEM_HDMI:
#endif
#ifdef FEATURE_FTM_EMMC
        case ITEM_EMMC:
#endif
#ifdef FEATURE_FTM_BATTERY   
        case ITEM_CHARGER:
#endif
#ifdef FEATURE_FTM_FLASH
        case ITEM_FLASH:
#endif
#ifdef FEATURE_FTM_RTC
        case ITEM_RTC:
#endif
#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
        case ITEM_FM:
#endif
#endif
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
        case ITEM_FMTX:
#endif
#endif
#endif
#ifdef FEATURE_FTM_SPK_OC
		case ITEM_SPK_OC:
#endif
#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
        case ITEM_BT:
#endif
#endif
#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
        case ITEM_WIFI:
#endif
#endif
#ifdef FEATURE_FTM_USB
        case ITEM_USB:
#endif
#ifdef MTK_GPS_SUPPORT 
#ifdef FEATURE_FTM_GPS
        case ITEM_GPS:
#endif
#endif
#ifdef FEATURE_FTM_MATV
        case ITEM_MATV_AUTOSCAN:
#endif
#ifdef WT_FTM_TP_OPEN_SHORT_CIRCUIT
		case ITEM_TP_OPENSHORT_CIRCUIT:
#endif
            if(item[i].value != FTM_TEST_PASS){
                return 0;
            }
            break;
        }
    }
    LOGD("%s:  success!, i = %d\n",__FUNCTION__, ITEM_MAX_IDS);

    return 1;
}

int isBoardTestSuccess(char ispcba)
{
    int ret = 0;
    int len = 0;
    char buf[2] = {0};

    FILE *fp = 0;
    if(ispcba == true){
        fp = fopen(pcbatestsuccessfilename, "r+");
    }else{
        fp = fopen(testsuccessfilename, "r+");
    }

    if(fp == NULL){
        LOGD("%s: open file fail \n",__FUNCTION__);
        return ret;
    }else{
        LOGD("%s: open file success \n",__FUNCTION__);
    }

    len = fread(buf , 1, 1, fp);
    if(len == 1){
        if(1 == buf[0]){
            ret = 1;
        }
    }
        
    fclose(fp);
    LOGD("%s: fclose(fp) ok",__FUNCTION__);
    return ret;
}


