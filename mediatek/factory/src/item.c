/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/ 
  

#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"
 
#include "item.h"

 
#define TAG        "[ITEM] "
  
 

item_t ftm_auto_test_items[] = {
#ifdef FEATURE_FTM_TOUCH
	item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif
		
#ifdef FEATURE_FTM_LCM
	item(ITEM_LCM,     uistr_lcm),
#endif
		
#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
	item(ITEM_SIGNALTEST, uistr_sig_test),
#endif
		
#ifdef FEATURE_FTM_BATTERY
	item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif
		
#ifdef FEATURE_FTM_FLASH
	item(ITEM_FLASH,   uistr_nand_flash),
#endif
		
#ifdef FEATURE_FTM_RTC
	item(ITEM_RTC,     uistr_rtc),
#endif
		
#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
	item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#endif
		
#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
	item(ITEM_BT, uistr_bluetooth),
#endif
#endif
		
#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
	item(ITEM_WIFI, uistr_wifi), //no uistr for wifi
#endif
#endif
		
#ifdef FEATURE_FTM_EMMC
	item(ITEM_EMMC,   uistr_emmc),
#endif
		
#ifdef FEATURE_FTM_MEMCARD
	item(ITEM_MEMCARD, uistr_memory_card),
#endif
		
#ifdef FEATURE_FTM_SIM
	item(ITEM_SIM, uistr_sim_detect),
#endif
		
#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
	item(ITEM_GPS,	   uistr_gps),
#endif
#endif
		
#ifdef FEATURE_FTM_MAIN_CAMERA
    item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif
#ifdef FEATURE_FTM_SUB_CAMERA
    item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif
		
		
		
#ifdef FEATURE_FTM_AUDIO
	item(ITEM_LOOPBACK_PHONEMICSPK,uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef RECEIVER_HEADSET_AUTOTEST
#ifdef FEATURE_FTM_AUDIO
	item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#endif
#ifdef FEATURE_FTM_MATV
	//item(ITEM_MATV_NORMAL,  "MATV HW Test"),
	item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
		
	item(ITEM_MAX_IDS, NULL),
		
};

item_t pc_control_items[] = {
    item(ITEM_FM,      "AT+FM"),
	item(ITEM_MEMCARD,      "AT+MEMCARD"),
	item(ITEM_SIM,      "AT+SIM"),
	item(ITEM_GPS,      "AT+GPS"),
	item(ITEM_EMMC,      "AT+EMMC"),
	item(ITEM_WIFI,	"AT+WIFI"),
	item(ITEM_LOOPBACK_PHONEMICSPK,      "AT+RINGTONE"),
	item(ITEM_SIGNALTEST,      "AT+SIGNALTEST"),
	item(ITEM_RTC,      "AT+RTC"),
	item(ITEM_CHARGER,      "AT+CHARGER"),
	item(ITEM_BT,      "AT+BT"),
	item(ITEM_MAIN_CAMERA, "AT+MAINCAMERA"),
	item(ITEM_SUB_CAMERA, "AT+SUBCAMERA"),
	item(ITEM_KEYS, "AT+KEY"),
	item(ITEM_MATV_AUTOSCAN, "AT+MATV"),
	item(ITEM_TOUCH_AUTO, "AT+TOUCH"),
#ifdef FEATURE_FTM_FLASH
	item(ITEM_CLRFLASH, "AT+FLASH"),
#endif
#ifdef FEATURE_FTM_EMMC
	item(ITEM_CLREMMC,"AT+FLASH"),
#endif
	item(ITEM_VIBRATOR, "AT+VIBRATOR"),
	item(ITEM_LED, "AT+LED"),
#ifdef FEATURE_FTM_RECEIVER
	item(ITEM_RECEIVER, "AT+RECEIVER"),
#endif
	item(ITEM_HEADSET, "AT+HEADSET"),
	item(ITEM_CMMB, "AT+CMMB"),
	item(ITEM_GSENSOR, "AT+GSENSOR"),
	item(ITEM_MSENSOR, "AT+MSENSOR"),
	item(ITEM_ALSPS, "AT+ALSPS"),
	item(ITEM_GYROSCOPE, "AT+GYROSCOPE"),
	item(ITEM_IDLE, "AT+IDLE"),
#ifdef FEATURE_FTM_LCM
	item(ITEM_LCM, "AT+LCM"),
#endif
#ifdef WT_FTM_HEADSET_MIC_RECEIVER_LOOPBACK
	item(ITEM_LOOPBACK_HMIC_RECEIVER, "AT+HMICRECV"),
#endif
#ifdef WT_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
	item(ITEM_LOOPBACK_HMIC_SPK, "AT+HMICSPK"),
#endif
#ifdef WT_FTM_HEADSET_MIC_EARPHONE_LOOPBACK
	item(ITEM_LOOPBACK_HMIC_EARPHONE, "AT+HMICEAR"),
#endif
#ifdef WT_FTM_PHONE_MIC_HEADSET_LOOPBACK
	item(ITEM_LOOPBACK_PMIC_HEADSET, "AT+PMICEAR"),
#endif
#ifdef WT_FTM_PHONE_REFMIC_HEADSET_LOOPBACK
	item(ITEM_LOOPBACK_PMIC2_HEADSET, "AT+PREFMICEAR"),
#endif
	item(ITEM_MAX_IDS, NULL),
};
item_t ftm_debug_test_items[] = {
	
#ifdef FEATURE_FTM_AUDIO
	item(ITEM_RECEIVER_DEBUG, uistr_info_audio_receiver_debug),
#endif	
	item(ITEM_MAX_IDS, NULL),
};


item_t ftm_test_items[] = {
    item(ITEM_VERSION,   uistr_version),
#ifdef FEATURE_FTM_KEYS
    item(ITEM_KEYS,    uistr_keys),
#endif
#ifdef FEATURE_FTM_JOGBALL
	item(ITEM_JOGBALL, uistr_jogball),
#endif
#ifdef FEATURE_FTM_OFN
	item(ITEM_OFN,     uistr_ofn),
#endif
#ifdef FEATURE_FTM_TOUCH
	item(ITEM_TOUCH,   uistr_touch),
	//item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif
#ifdef FEATURE_FTM_LCD
#ifdef FEATURE_FTM_VIBRATOR
	item(ITEM_LCD,	   uistr_backlight_level1),
#else
	item(ITEM_LCD,     uistr_backlight_level),
#endif
#endif
		
#ifdef FEATURE_FTM_LCM
	item(ITEM_LCM,     uistr_lcm),
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
	item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif
#ifdef FEATURE_FTM_MAIN2_CAMERA
	item(ITEM_MAIN2_CAMERA,  uistr_main2_sensor),
#endif
#ifdef FEATURE_FTM_SUB_CAMERA
	item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif
		
#ifdef FEATURE_FTM_STROBE
	item(ITEM_STROBE, uistr_strobe),
#endif		
	//#ifdef FEATURE_FTM_SIGNALTEST
#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
	//item(ITEM_SIGNALTEST, uistr_sig_test),
#endif
		//#endif
#ifdef FEATURE_FTM_VIBRATOR
	item(ITEM_VIBRATOR, uistr_vibrator),
#endif
#ifdef FEATURE_FTM_LED
	item(ITEM_LED,     uistr_led),
#endif
#ifdef FEATURE_FTM_AUDIO
	//item(ITEM_LOOPBACK_PHONEMICSPK, uistr_info_audio_loopback_phone_mic_speaker),
#ifdef FEATURE_FTM_RECEIVER
	item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_RECEIVER_LOOPBACK
	item(ITEM_LOOPBACK, uistr_info_audio_loopback),
#endif
#ifdef FEATURE_FTM_ACSLB
	//item(ITEM_ACOUSTICLOOPBACK, uistr_info_audio_acoustic_loopback),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_HEADSET_LOOPBACK
	item(ITEM_LOOPBACK1, uistr_info_audio_loopback_phone_mic_headset),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_SPEAKER_LOOPBACK
#ifdef MTK_AUDIO_EXTCODEC_SUPPORT
	item(ITEM_LOOPBACK2_MIC1, uistr_info_audio_loopback_phone_mic1_speaker),
	item(ITEM_LOOPBACK2_MIC2, uistr_info_audio_loopback_phone_mic2_speaker),
#else
	item(ITEM_LOOPBACK2, uistr_info_audio_loopback_phone_mic_speaker),
#endif
#endif
#ifdef FEATURE_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
	item(ITEM_LOOPBACK3, uistr_info_audio_loopback_headset_mic_speaker),
#endif
#ifdef FEATURE_FTM_WAVE_PLAYBACK
	item(ITEM_WAVEPLAYBACK, uistr_info_audio_loopback_waveplayback),
#endif
#endif //FEATURE_FTM_AUDIO
		
#ifdef FEATURE_FTM_HEADSET
	item(ITEM_HEADSET, uistr_info_headset),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
	item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
	item(ITEM_FMTX, uistr_info_fmt_title),
#endif
#endif
#endif

#ifdef FEATURE_FTM_SPK_OC
	item(ITEM_SPK_OC, uistr_info_speaker_oc),
#endif
#ifdef FEATURE_FTM_OTG
	item(ITEM_OTG, "OTG"),
#endif
#ifdef FEATURE_FTM_USB
	item(ITEM_USB, "USB"),
#endif
#ifdef CUSTOM_KERNEL_ACCELEROMETER
	//item(ITEM_GS_CALI, uistr_g_sensor_c),
	item(ITEM_GSENSOR, uistr_g_sensor),
#endif
#ifdef CUSTOM_KERNEL_MAGNETOMETER
	item(ITEM_MSENSOR, uistr_m_sensor),
#endif
#ifdef CUSTOM_KERNEL_ALSPS
	item(ITEM_PS_CALI, uistr_ps_cal),
	item(ITEM_ALSPS, uistr_als_ps),
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
	item(ITEM_BAROMETER, uistr_barometer),
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
	item(ITEM_GYROSCOPE, uistr_gyroscope),
	item(ITEM_GYROSCOPE_CALI, uistr_gyroscope_c),
#endif
#ifdef WT_BU52051NVX_MHALL_SUPPORT
	item(ITEM_MHALL_SENSOR, uistr_mhall_sensor),
#endif
		
#ifdef MTK_NFC_SUPPORT
	item(ITEM_NFC,    uistr_nfc),
#endif
			
		
#if 1
#ifdef FEATURE_FTM_MATV
	item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
		
#if 0
	//item(ITEM_MATV_NORMAL,  "MATV HW Test"),
	item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#endif

#ifdef FEATURE_FTM_HDMI
	item(ITEM_HDMI, "HDMI"),
#endif

#ifdef FEATURE_FTM_BATTERY
	//item(ITEM_CHARGER, "Battery & Charger"),
	item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif
#ifdef FEATURE_FTM_IDLE
	//item(ITEM_IDLE,    uistr_idle),
#endif
#ifdef FEATURE_FTM_TVOUT
	item(ITEM_TVOUT,     uistr_info_tvout_item),
#endif
#ifdef FEATURE_FTM_CMMB
	item(ITEM_CMMB, uistr_cmmb),
#endif
#ifdef FEATURE_FTM_EMI
	item(ITEM_EMI, uistr_system_stability),
#endif
#ifdef FEATURE_FTM_RTC
	item(ITEM_RTC,     uistr_rtc),
#endif
		

/*=========background auto test start here==========*/
#ifdef FEATURE_FTM_FLASH
	item(ITEM_FLASH,   uistr_nand_flash),
#endif
#ifdef FEATURE_FTM_EMMC
	item(ITEM_EMMC,   uistr_emmc),
#endif
#ifndef FEATURE_FTM_WIFI_ONLY
#ifdef FEATURE_FTM_MEMCARD
	item(ITEM_MEMCARD, uistr_memory_card),
#endif
#ifdef FEATURE_FTM_SIMCARD
	item(ITEM_SIMCARD, uistr_sim_card),
#endif
#ifdef FEATURE_FTM_SIM
	item(ITEM_SIM, uistr_sim_detect),
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
	item(ITEM_BT, uistr_bluetooth),
#endif
#endif
		
#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
	item(ITEM_WIFI, uistr_wifi),
#endif
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
	item(ITEM_GPS,     uistr_gps),
#endif
#endif

	item(ITEM_MAX_IDS, NULL),
};

// sunxuewen added for htc cid 20140418
item_t ftm_htc_cid_select_items[] = 
{
	item_cid(ITEM_HTC_CID_English_001,         uistr_CID_English_001   ),
	item_cid(ITEM_HTC_CID_German_102,          uistr_CID_German_102    ),
	item_cid(ITEM_HTC_CID_French_203,          uistr_CID_French_203    ),
	item_cid(ITEM_HTC_CID_Spanish_304,         uistr_CID_Spanish_304   ),
	item_cid(ITEM_HTC_CID_Italian_405,         uistr_CID_Italian_405   ),
	item_cid(ITEM_HTC_CID_Portuguess_506,      uistr_CID_Portuguess_506),
	item_cid(ITEM_HTC_CID_Russian_A07,         uistr_CID_Russian_A07   ),
	item_cid(ITEM_HTC_CID_Danish_F08,          uistr_CID_Danish_F08    ),
	item_cid(ITEM_HTC_CID_Swedish_G09,         uistr_CID_Swedish_G09   ),
	item_cid(ITEM_HTC_CID_Norwegian_H10,       uistr_CID_Norwegian_H10 ),
	item_cid(ITEM_HTC_CID_Dutch_E11,           uistr_CID_Dutch_E11     ),
	item_cid(ITEM_HTC_CID_European_Z12,        uistr_CID_European_Z12  ),
	item_cid(ITEM_HTC_CID_Nordic_Y13,          uistr_CID_Nordic_Y13    ),
	item_cid(ITEM_HTC_CID_Finnish_I14,         uistr_CID_Finnish_I14   ),
	item_cid(ITEM_HTC_CID_Arabic_J15,          uistr_CID_Arabic_J15    ),
	item_cid(ITEM_HTC_CID_English_016,         uistr_CID_English_016   ),
	item_cid(ITEM_HTC_CID_English_017,         uistr_CID_English_017   ),
	item_cid(ITEM_HTC_CID_Hebrew_K18,          uistr_CID_Hebrew_K18    ),
	item_cid(ITEM_HTC_CID_English_019,         uistr_CID_English_019   ),
	item_cid(ITEM_HTC_CID_Japanese_820,        uistr_CID_Japanese_820  ),
	item_cid(ITEM_HTC_CID_T_Chinese_621,       uistr_CID_T_Chinese_621 ),
	item_cid(ITEM_HTC_CID_T_Chinese_622,       uistr_CID_T_Chinese_622 ),
	item_cid(ITEM_HTC_CID_English_023,         uistr_CID_English_023   ),
	item_cid(ITEM_HTC_CID_Czech_C24,           uistr_CID_Czech_C24     ),
	item_cid(ITEM_HTC_CID_Polish_B25,          uistr_CID_Polish_B25    ),
	item_cid(ITEM_HTC_CID_English_026,         uistr_CID_English_026   ),
	item_cid(ITEM_HTC_CID_Turkish_M27,         uistr_CID_Turkish_M27   ),
	item_cid(ITEM_HTC_CID_English_028,         uistr_CID_English_028   ),
	item_cid(ITEM_HTC_CID_English_029,         uistr_CID_English_029   ),
	item_cid(ITEM_HTC_CID_English_030,         uistr_CID_English_030   ),
	item_cid(ITEM_HTC_CID_Spanish_331,         uistr_CID_Spanish_331   ),
	item_cid(ITEM_HTC_CID_English_032,         uistr_CID_English_032   ),
	item_cid(ITEM_HTC_CID_Turkish_M33,         uistr_CID_Turkish_M33   ),
	item_cid(ITEM_HTC_CID_Greek_N34,           uistr_CID_Greek_N34     ),
	item_cid(ITEM_HTC_CID_Dutch_E35,           uistr_CID_Dutch_E35     ),
	item_cid(ITEM_HTC_CID_Portuguess_D36,      uistr_CID_Portuguess_D36),
	item_cid(ITEM_HTC_CID_English_037,         uistr_CID_English_037   ),
	item_cid(ITEM_HTC_CID_English_038,         uistr_CID_English_038   ),
	item_cid(ITEM_HTC_CID_German_139,          uistr_CID_German_139    ),
	item_cid(ITEM_HTC_CID_German_140,          uistr_CID_German_140    ),
	item_cid(ITEM_HTC_CID_Dutch_E41,           uistr_CID_Dutch_E41     ),
	item_cid(ITEM_HTC_CID_Arabic_J42,          uistr_CID_Arabic_J42    ),
	item_cid(ITEM_HTC_CID_English_043,         uistr_CID_English_043   ),
	item_cid(ITEM_HTC_CID_English_044,         uistr_CID_English_044   ),
	item_cid(ITEM_HTC_CID_Arabic_J45,          uistr_CID_Arabic_J45    ),
	item_cid(ITEM_HTC_CID_EnglishUK_046,       uistr_CID_EnglishUK_046 ),
	item_cid(ITEM_HTC_CID_French_247,          uistr_CID_French_247    ),
	item_cid(ITEM_HTC_CID_Arabic_J48,          uistr_CID_Arabic_J48    ),
	item_cid(ITEM_HTC_CID_Russian_A48,         uistr_CID_Russian_A48   ),
	item_cid(ITEM_HTC_CID_French_249,          uistr_CID_French_249    ),
	item_cid(ITEM_HTC_CID_Spanish_350,         uistr_CID_Spanish_350   ),
	item_cid(ITEM_HTC_CID_Polish_B51,          uistr_CID_Polish_B51    ),
	item_cid(ITEM_HTC_CID_English_052,         uistr_CID_English_052   ),
	item_cid(ITEM_HTC_CID_Balkans_056,         uistr_CID_Balkans_056   ),
	item_cid(ITEM_HTC_CID_Myanmar_058,         uistr_CID_Myanmar_058   ),
	item_cid(ITEM_HTC_CID_Vietnam_059,         uistr_CID_Vietnam_059   ),
	//item_cid(ITEM_HTC_CID_DEVICE_INFO,         uistr_CID_Device_Info   ),
	item_cid(ITEM_MAX_IDS, NULL)
};
// sunxuewen added end


item_t ftm_cust_items[ITEM_MAX_IDS];
item_t ftm_cust_auto_items[ITEM_MAX_IDS];
 

item_t *get_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_item_list");

	items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	return items;
}

item_t *get_debug_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_debug_item_list");

	items = ftm_debug_test_items;

	return items;
}

item_t *get_manual_item_list(void)
{
	item_t *items;
	item_t *items_auto;
	int i = 0;
	int j =0;
	LOGD(TAG "get_manual_item_list");

	items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	items_auto = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

	while (items_auto[i].name)
	{
		for(j =0;items[j].name != NULL ;j++)
		{
			if(strcmp(items[j].name,items_auto[i].name)==0)
			{
				items[j].mode = FTM_AUTO_ITEM;
				LOGD(TAG "%s",items[j].name);
			}
		}
		i++;
	}

	return items;
}

item_t *get_auto_item_list(void)
{
	item_t *items;

	items = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

	return items;
}

const char *get_item_name(item_t *item, int id)
{
	int i;

	while (item->name) {
		if (item->id == id)
			return item->name;
		item++;
	}
	return NULL;
}

int get_item_id(item_t *item, char *name)
{
	int i;

	while (item->name)
	{
		if(strlen(item->name)==strlen(name))
		{
			if (!strncasecmp(item->name, name, strlen(item->name)))
				return item->id;
		}
		item++;
	}
	return -1;
}




   
