/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
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
#include <cutils/properties.h>
#include <unistd.h>
#include <pthread.h>


#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"
#include "cid.h"

//#include "libnvram.h"
//#include "CFG_file_info_custom.h"

#include "item.h"

#include "report_save.h"

#define TAG        "[FACTORY] "

#define BATT_CAPACITY_FILE "/sys/class/power_supply/battery/capacity"
bool isBattLowLevel = false;

#define FTM_CUST_FILE1  "/sdcard/factory.ini"
#define FTM_CUST_FILE2  "/etc/factory.ini"

#define START "AT+START"
#define STOP "AT+STOP"
#define REQUEST_DATA "AT+REQUESTDATA"
#define VERSION "AT+VERSION"
#define READ_BARCODE "AT+READBARCODE"
#define WRITE_BARCODE "AT+WRITEBARCODE"
#define VIBRATOR_ENABLE "/sys/class/timed_output/vibrator/enable"

#ifdef FEATURE_FTM_AUDIO_AUTO_TEST_SUPPORT
#ifdef WT_FTM_PHONE_MIC_HEADSET_LOOPBACK
#define PMIC_HEADSET_LOOPBACK_STOP "AT+PMICEAR=0"
#endif
#ifdef WT_FTM_PHONE_REFMIC_HEADSET_LOOPBACK
#define PREFMIC_HEADSET_LOOPBACK_STOP "AT+PREFMICEAR=0"
#endif
#ifdef WT_FTM_HEADSET_MIC_RECEIVER_LOOPBACK
#define HMIC_REC_LOOPBACK_STOP "AT+HMICRECV=0"
#endif
#ifdef WT_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
#define HMIC_SPK_LOOPBACK_STOP "AT+HMICSPK=0"
#endif
#ifdef WT_FTM_HEADSET_MIC_EARPHONE_LOOPBACK
#define HMIC_EARPHONE_LOOPBACK_STOP "AT+HMICEAR=0"
#endif

int gAudio_auto_test = 0;
#endif

#define FEATURE_FTM_CPU_INFO_SUPPORT
#define FEATURE_FTM_ROM_CAPACITY
#define FEATURE_FTM_RAM_CAPACITY

// add for idle current auto test
int usb_com_port;// = -1;//zhaoqf_sh modify
int usb_status = 0;
int usb_plug_in = 1;
int need_get_batt_data = 0;
int batt_volt = 0;
int charging_current = 0;
int sIsBoardTest = 0;
int s_pcba_test_success = 0;
int g_pcba_test = false;

int display_version(int autoreturn);

pthread_mutex_t at_command_mutex = PTHREAD_MUTEX_INITIALIZER;

//#define ENTER_TP_TEST

#define WT_SN_MAX_COUNT 24//Display SN Num
static item_t ftm_menu_items[] = {
    //item(ITEM_MUI_TEST,"Mini-UI Test"),
#if 1
    item(ITEM_MMI_TEST, uistr_mmi_test),
    item(ITEM_PCBA_TEST, uistr_pcba_test),
#ifdef HTC_CID
    item(ITEM_HTC_CID_SELETC, uistr_htc_cid_select), // sunxuewen added
#endif
#else    
#ifdef FEATURE_MTK_ORIGINAL
    item(ITEM_AUTO_TEST, uistr_auto_test),
#endif    
    item(ITEM_FULL_TEST, uistr_full_test),
    item(ITEM_ITEM_TEST, uistr_item_test),
    item(ITEM_REPORT,    uistr_test_report),
#ifdef FEATURE_MTK_ORIGINAL
    item(ITEM_DEBUG_TEST,uistr_debug_test),
#endif    
#ifdef FEATURE_FTM_CLEARFLASH
    item(ITEM_CLRFLASH,  uistr_clear_flash),
#endif
#ifdef FEATURE_FTM_CLEAREMMC
    item(ITEM_CLREMMC,  uistr_clear_emmc),
#endif
#endif
//    item(ITEM_SN,    uistr_info_sn),
    item(ITEM_HARDWARE_INFO,   uistr_hardware_info),
//    item(ITEM_VERSION,   uistr_version),
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    item(ITEM_GS_CALI, uistr_g_sensor_c),
#endif    
#ifdef FEATURE_MTK_ORIGINAL
    item(ITEM_REBOOT,    uistr_reboot),
#endif
    item(ITEM_POWEROFF,    uistr_poweroff),
    item(ITEM_MAX_IDS,   NULL),
};

static item_t ftm_menu_items_pcba[] = {
    item(ITEM_PCBA_FULL_TEST, uistr_full_test_pcba),
    item(ITEM_PCBA_ITEM_TEST, uistr_item_test_pcba),
    item(ITEM_REPORT,    uistr_pcba_test_report),
    item(ITEM_MAX_IDS,   NULL),
};

static item_t ftm_menu_items_mmi[] = {
    item(ITEM_FULL_TEST, uistr_full_test_mmi),
    item(ITEM_ITEM_TEST, uistr_item_test_mmi),
    item(ITEM_REPORT,    uistr_mmi_test_report),
    item(ITEM_MAX_IDS,   NULL),
};

extern item_t ftm_auto_test_items[];
extern item_t pc_control_items[]; 
extern item_t ftm_debug_test_items[];
extern item_t ftm_test_items[];
extern item_t ftm_cust_items[ITEM_MAX_IDS];
extern item_t ftm_cust_auto_items[ITEM_MAX_IDS];
// sunxuewen added for htc_cid select
extern item_t ftm_htc_cid_select_items[];

struct ftm_htc_cid_table
{
	int htc_cid_item;
	char *htc_cid_value;
} CID_TABLE[] = {
		{ITEM_HTC_CID_English_001,    "HTC__001" },
		{ITEM_HTC_CID_German_102,     "HTC__102" },
		{ITEM_HTC_CID_French_203,     "HTC__203" },
		{ITEM_HTC_CID_Spanish_304,    "HTC__304" },
		{ITEM_HTC_CID_Italian_405,    "HTC__405" },
		{ITEM_HTC_CID_Portuguess_506, "HTC__506" },
		{ITEM_HTC_CID_Russian_A07,    "HTC__A07" },
		{ITEM_HTC_CID_Danish_F08,     "HTC__F08" },
		{ITEM_HTC_CID_Swedish_G09,    "HTC__G09" },
		{ITEM_HTC_CID_Norwegian_H10,  "HTC__H10" },
		{ITEM_HTC_CID_Dutch_E11,      "HTC__E11" },
		{ITEM_HTC_CID_European_Z12,   "HTC__Z12" },
		{ITEM_HTC_CID_Nordic_Y13,     "HTC__Y13" },
		{ITEM_HTC_CID_Finnish_I14,    "HTC__I14" },
		{ITEM_HTC_CID_Arabic_J15,     "HTC__J15" },
		{ITEM_HTC_CID_English_016,    "HTC__016" },
		{ITEM_HTC_CID_English_017,    "HTC__017" },
		{ITEM_HTC_CID_Hebrew_K18,     "HTC__K18" },	
		{ITEM_HTC_CID_English_019,    "HTC__019" },
		{ITEM_HTC_CID_Japanese_820,   "HTC__820" },
		{ITEM_HTC_CID_T_Chinese_621,  "HTC__621" },
		{ITEM_HTC_CID_T_Chinese_622,  "HTC__622" },
		{ITEM_HTC_CID_English_023,    "HTC__023" },
		{ITEM_HTC_CID_Czech_C24,      "HTC__C24" },
		{ITEM_HTC_CID_Polish_B25,     "HTC__B25" },
		{ITEM_HTC_CID_English_026,    "HTC__026" },
		{ITEM_HTC_CID_Turkish_M27,    "HTC__M27" },	
		{ITEM_HTC_CID_English_028,    "HTC__028" },
		{ITEM_HTC_CID_English_029,    "HTC__029" },
		{ITEM_HTC_CID_English_030,    "HTC__030" },
		{ITEM_HTC_CID_Spanish_331,    "HTC__331" },
		{ITEM_HTC_CID_English_032,    "HTC__032" },
		{ITEM_HTC_CID_Turkish_M33,    "HTC__M33" },
		{ITEM_HTC_CID_Greek_N34,      "HTC__N34" },
		{ITEM_HTC_CID_Dutch_E35,      "HTC__E35" },
		{ITEM_HTC_CID_Portuguess_D36, "HTC__D36" },	
		{ITEM_HTC_CID_English_037,    "HTC__037" },
		{ITEM_HTC_CID_English_038,    "HTC__038" },
		{ITEM_HTC_CID_German_139,     "HTC__139" },
		{ITEM_HTC_CID_German_140,     "HTC__140" },
		{ITEM_HTC_CID_Dutch_E41,      "HTC__E41" },
		{ITEM_HTC_CID_Arabic_J42,     "HTC__J42" },
		{ITEM_HTC_CID_English_043,    "HTC__043" },
		{ITEM_HTC_CID_English_044,    "HTC__044" },
		{ITEM_HTC_CID_Arabic_J45,     "HTC__J45" },
		{ITEM_HTC_CID_EnglishUK_046,  "HTC__046" },
		{ITEM_HTC_CID_French_247,     "HTC__247" },
		{ITEM_HTC_CID_Arabic_J48,     "HTC__J48" },
		{ITEM_HTC_CID_Russian_A48,    "HTC__A48" },
		{ITEM_HTC_CID_French_249,     "HTC__249" },
		{ITEM_HTC_CID_Spanish_350,    "HTC__350" },
		{ITEM_HTC_CID_Polish_B51,     "HTC__B51" },
		{ITEM_HTC_CID_English_052,    "HTC__052" },
		{ITEM_HTC_CID_Balkans_056,    "HTC__056" },
		{ITEM_HTC_CID_Myanmar_058,    "HTC__058" },
		{ITEM_HTC_CID_Vietnam_059,    "HTC__059" },
};
// sunxuewen added end


char at_command[128] = {0};
#ifdef FEATURE_FTM_VIBRATOR
extern bool vibrator_test_exit;
#endif
#ifdef FEATURE_FTM_LED
extern bool keypadled_test_exit;
extern bool led_test_exit;
#endif


int g_nr_lines = 0;
char test_data[128];
int status = 0;


sp_ata_data return_data;
// add for idle current auto test

static int get_AT_command(char *origin_at_command)
{
    char *ptr = NULL;
	char *p;
	char *temp_at_command = origin_at_command;
	int result = 0;
	int i = 0;
	int len = strlen(origin_at_command);
	p = origin_at_command;
	ptr = strchr(temp_at_command, '=');
	if(ptr == NULL)
	{
	    LOGD(TAG "ptr is null\n");
        pthread_mutex_lock (&at_command_mutex);
		strcpy(at_command, origin_at_command);
        pthread_mutex_unlock (&at_command_mutex);
		result = 0;
	}
	else
	{
		if(!strncmp(++ptr, "CLOSE", strlen("CLOSE")))
		{
			*(--ptr) = '\0';

			result = 1;
		}
		else
		{
	    	for (i = 0; i < len; i++, p++)
			{
           		if ((*p == '+') && ((i + 1) < len))
				{
                *p = '\0';
                break;
            }
        }
	    LOGD(TAG "ptr is not null\n");
//		strcpy(at_command, ++p);
        *(--ptr) = '\0';
		ftm_set_prop(++p, ++ptr);
			result = 2;
        }

	}
//    LOGD(TAG "%s\n");
	return result;
}



int get_is_ata(){
    LOGD(TAG "status........................... = %d\n", status);
    return status;
}

static int item_test_report(item_t *items, char *buf, int bufsz)
{
    int    num;
    int    chosen_item;
    bool   quit;
    struct itemview triv; /* test report item view */
    item_t rpt_items[ITEM_MAX_IDS + 1];
    text_t tr_title;
    struct ftm_param param;
    struct ftm_module *mod = NULL; 
    int ret;
    
    init_text(&tr_title, uistr_test_report, COLOR_YELLOW);

    ui_init_itemview(&triv);

    quit = false;
    memset(rpt_items, 0, sizeof(item_t) * (ITEM_MAX_IDS + 1));
    num = create_report(items, rpt_items, ITEM_MAX_IDS, buf, bufsz);
    triv.set_title(&triv, &tr_title);
    triv.set_items(&triv, rpt_items, 0);
    while (!quit) {
        chosen_item = triv.run(&triv, &quit);
		if(chosen_item == ITEM_ID_SEPRATE_FLAG)
		{
			continue;
		}
        if (chosen_item == ITEM_MAX_IDS)
            break;
        if(!quit){
            param.name = get_item_name(items, chosen_item);
            ftm_entry(chosen_item, &param);
            mod = ftm_get_module(chosen_item);
            ret = report_writeByID(mod->id, mod->test_result);
            if(-1== ret){
            LOGD(TAG "%s insert ret =%d\n", __FUNCTION__, ret);
            }else{
            }
            create_report(items, rpt_items, ITEM_MAX_IDS, buf, bufsz);
        }
    }
    return 0;
}

/***************************Add by zhaoqf_sh Start************************************/
int is_full_mode = 0;
enum {
    ITEM_PASS,
    ITEM_FAIL
};
static item_t result_sel_item[] = {
    item(ITEM_PASS,             "Test Pass"),
    item(ITEM_FAIL,             "Test Fail"),
    item(-1, NULL),
};

char barcode[128] = "unknown";
char barcode2[128] = "unknown";
char calibrationInfo[128] = "unknown";
char imei1[128]={0};
char imei2[128]={0};
char sw_edition[128] = {0};
char boardtest[256] = {0};
#ifdef FEATURE_FTM_BT
extern bool bt_test_pass;
#endif
#ifdef FEATURE_FTM_WIFI
extern bool wifi_test_pass; 
#endif
extern bool sim_test_pass;
#ifdef FEATURE_FTM_GPS
extern bool gps_test_pass;
#endif
bool BT_is_testing = false;
bool BT_test_canceled = false;
bool GPS_is_testing = false;
bool wifi_is_testing = false;
bool wifi_test_canceled = false;
bool background_test_complete = false;
bool GSM_bt_cal_pass = false;
bool TD_bt_cal_pass = false;
bool WCDMA_bt_cal_pass = false;



static void initbackrunparam(void)
{
	BT_is_testing = false;
	BT_test_canceled = false;
#ifdef FEATURE_FTM_BT	
	bt_test_pass = false;
#endif
	wifi_is_testing = false;
	wifi_test_canceled = false;
#ifdef FEATURE_FTM_WIFI	
	wifi_test_pass = false;
#endif
	sim_test_pass = false;
    GPS_is_testing = false;	
#ifdef FEATURE_FTM_GPS	
    gps_test_pass = false;
#endif
}

int get_rf_test_and_calibration_info(int fd,char *result)
{
	const int BUF_SIZE = 128;
	char buf[BUF_SIZE];
	memset(buf,'\0',BUF_SIZE);
	const int HALT_TIME = 100 * 1000;
	int count = 0;
	int i=0;
	strcpy(buf, "AT+WRFTCAL\r\n");
	
	strcpy(result, "unknow\n");
retry:
	send_at(fd, buf);
	memset(buf,'\0',BUF_SIZE);
	read_ack(fd,buf,BUF_SIZE);
	LOGD("buf %s",buf);
	char *p = NULL;
	p = buf;
	if(p) 
	{
		strcpy(result, p);
		count = 3;
	} 
	else 
	{
		strcpy(buf, "AT\r\n");
		count++;
	}
	if(count < 3)
	{
        goto retry;
	}

	LOGE(TAG "get_rf_test_and_calibration_info result = %s\n", result);
	return 0;
}

//zhaoqf_sh add for SN display
static int create_SNinfo(char *info, int size)
{
    char modem_ver[128] = "unknown";
    char modem_ver2[128] = "unknown";    
	int len = 0;
    int fd = -1;
    int fd2 = -1;
    int i = 0;
    int reslt;    
    char *ptr= NULL;  
    char SN1[64] = {0};    
    char SN2[64] = {0};	
    
	g_nr_lines = 0;
		
#ifdef MTK_ENABLE_MD1
	fd= openDevice();
	if(-1 == fd) {
 		LOGD(TAG "Fail to open CCCI interface\n");
		return 0;
       }
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd, "AT\r\n");
	wait4_ack (fd, NULL, 3000);
#endif

#ifdef MTK_ENABLE_MD2
	fd2= openDeviceWithDeviceName("/dev/ccci2_tty0");
	if(-1 == fd2) {
		LOGD(TAG "Fail to open ttyMT0 interface\n");
		return 0;
	}
	initTermIO(fd2,5);
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd2, "AT\r\n");
	wait4_ack (fd2, NULL, 3000);
#endif

#ifdef MTK_ENABLE_MD1
    reslt = getModemVersion(fd,modem_ver);
    ptr = strchr(modem_ver, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver[strlen(modem_ver)-1] == '\r')
	{
        modem_ver[strlen(modem_ver)-1] = 0;
	}

    reslt = getBarcode(fd,barcode);
    ptr = strchr(barcode, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode) <= 0)
		strcpy(barcode, "unknown");
	closeDevice(fd);
#endif

#if defined(MTK_ENABLE_MD2)
    reslt = getModemVersion(fd2,modem_ver2);
    ptr = strchr(modem_ver2, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver2[strlen(modem_ver2)-1] == '\r')
	{
        modem_ver2[strlen(modem_ver2)-1] = 0;
	}        
    reslt = getBarcode(fd2,barcode2);
    ptr = strchr(barcode2, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode2) <= 0)
		strcpy(barcode2, "unknown");
    closeDevice(fd2);
#endif

#ifdef MTK_ENABLE_MD1
    strncpy(SN1, barcode, WT_SN_MAX_COUNT);
	ptr = strchr(SN1, ' ');
	if (ptr != NULL)
	{
		*ptr = 0;
	}

    print_verinfo(info, &len,  "SN", SN1);//barcode
#elif defined(MTK_ENABLE_MD2)
    strncpy(SN2, barcode2, WT_SN_MAX_COUNT);
	ptr = strchr(SN2, ' ');
	if (ptr != NULL)
	{
		*ptr = 0;
	}

    print_verinfo(info, &len,  "SN", SN2);//barcode2
#endif

    return 0;
}
static int create_SoftWareinfo(char *info, int size)
{
    char modem_ver[128] = "unknown";
    char modem_ver2[128] = "unknown";    
	int len = 0;
    int fd = -1;
    int fd2 = -1;
    int i = 0;
    int reslt;    
    char *ptr= NULL;  
    char SN1[64] = {0};    
    char SN2[64] = {0};	
	int msg_len = 0;
	char val_sw[128]={0};
    char val_GSM_BT[128]={0};
    char val_TD_WCDMA_BT[128]={0}; 	

	char Phone_SN[64] = {0};	
    
	g_nr_lines = 0;

	property_get("ro.build.display.wtid", val_sw, "unknown");
	print_verinfo(info, &len,  "SW Ver", val_sw);
		
#ifdef MTK_ENABLE_MD1
	fd= openDevice();
	if(-1 == fd) {
 		LOGD(TAG "Fail to open CCCI interface\n");
		return 0;
       }
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd, "AT\r\n");
	wait4_ack (fd, NULL, 3000);
#endif

#ifdef MTK_ENABLE_MD2
	fd2= openDeviceWithDeviceName("/dev/ccci2_tty0");
	if(-1 == fd2) {
		LOGD(TAG "Fail to open ttyMT0 interface\n");
		return 0;
	}
	initTermIO(fd2,5);
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd2, "AT\r\n");
	wait4_ack (fd2, NULL, 3000);
#endif

#if defined(MTK_ENABLE_MD1)
	getIMEI(1, fd, imei1);
	#ifdef GEMINI
		#if defined(MTK_ENABLE_MD2)
			getIMEI(1, fd2, imei2);
		#else
			getIMEI(2, fd, imei2);
		#endif
	#endif
#elif defined(MTK_ENABLE_MD2)
	getIMEI(1, fd2, imei1);
	#ifdef GEMINI
		getIMEI(2, fd2, imei2);
	#endif
#endif

	print_verinfo(info, &len,  "IMEI1", imei1);
#ifdef GEMINI
	print_verinfo(info, &len,  "IMEI2", imei2);
#endif

#ifdef MTK_ENABLE_MD1
    reslt = getModemVersion(fd,modem_ver);
    ptr = strchr(modem_ver, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver[strlen(modem_ver)-1] == '\r')
	{
        modem_ver[strlen(modem_ver)-1] = 0;
	}

    reslt = getBarcode(fd,barcode);
    ptr = strchr(barcode, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode) <= 0)
		strcpy(barcode, "unknown");


    //zhaoqf_sh get cal info for GSM/TD 
    reslt = get_rf_test_and_calibration_info(fd,calibrationInfo);

	for(i=0; i<128; i++)
	{
		LOGD(TAG "calibrationInfo_MD1[%d]=%2x\n", i, calibrationInfo[i]);
	}
	
    //judge GSM Calibration is ok or not
    if(((0xe8 == calibrationInfo[4]) && (0x03 == calibrationInfo[5]))
		&& ((0xe8 == calibrationInfo[2]) && (0x03 == calibrationInfo[3])))
    {
        GSM_bt_cal_pass = true;
    }

    //judge TD Calibration is ok or not
    if(((0xe8 == calibrationInfo[8]) && (0x03 == calibrationInfo[9])) 
		&& ((0xe8 == calibrationInfo[6]) && (0x03 == calibrationInfo[7])))
    {
        TD_bt_cal_pass = true;
    }

    //judge WCDMA Calibration is ok or not
    if(((0xe8 == calibrationInfo[12]) && (0x03 == calibrationInfo[13])) 
		&& ((0xe8 == calibrationInfo[10]) && (0x03 == calibrationInfo[11])))
    {
        WCDMA_bt_cal_pass = true;
    }
	
	closeDevice(fd);
#endif

#if defined(MTK_ENABLE_MD2)
    reslt = getModemVersion(fd2,modem_ver2);
    ptr = strchr(modem_ver2, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver2[strlen(modem_ver2)-1] == '\r')
	{
        modem_ver2[strlen(modem_ver2)-1] = 0;
	}        
    reslt = getBarcode(fd2,barcode2);
    ptr = strchr(barcode2, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode2) <= 0)
		strcpy(barcode2, "unknown");


    //zhaoqf_sh get cal info for GSM/TD 
    reslt = get_rf_test_and_calibration_info(fd2,calibrationInfo);

	for(i=0; i<128; i++)
	{
		LOGD(TAG "calibrationInfo_MD2[%d]=%2x\n", i, calibrationInfo[i]);
	}
	
    //judge GSM Calibration is ok or not
    if(((0xe8 == calibrationInfo[4]) && (0x03 == calibrationInfo[5]))
		&& ((0xe8 == calibrationInfo[2]) && (0x03 == calibrationInfo[3])))
    {
        GSM_bt_cal_pass = true;
    }

    //judge TD Calibration is ok or not
    if(((0xe8 == calibrationInfo[8]) && (0x03 == calibrationInfo[9])) 
		&& ((0xe8 == calibrationInfo[6]) && (0x03 == calibrationInfo[7])))
    {
        TD_bt_cal_pass = true;
    }

    //judge WCDMA Calibration is ok or not
    if(((0xe8 == calibrationInfo[12]) && (0x03 == calibrationInfo[13])) 
		&& ((0xe8 == calibrationInfo[10]) && (0x03 == calibrationInfo[11])))
    {
        WCDMA_bt_cal_pass = true;
    }
	
    closeDevice(fd2);
#endif

#ifdef MTK_ENABLE_MD1
    strncpy(SN1, barcode, WT_SN_MAX_COUNT);
	ptr = strchr(SN1, ' ');
	if (ptr != NULL)
	{
		*ptr = 0;
	}

    print_verinfo(info, &len,  "SN", SN1);//barcode
#elif defined(MTK_ENABLE_MD2)
    strncpy(SN2, barcode2, WT_SN_MAX_COUNT);
	ptr = strchr(SN2, ' ');
	if (ptr != NULL)
	{
		*ptr = 0;
	}

    print_verinfo(info, &len,  "SN", SN2);//barcode2
#endif

    memset(Phone_SN, 0, sizeof(Phone_SN)); 
#if defined(MTK_ENABLE_MD1)
    strncpy(Phone_SN, &barcode[30], WT_SN_MAX_COUNT); 
#elif defined(MTK_ENABLE_MD2)
    strncpy(Phone_SN, &barcode2[30], WT_SN_MAX_COUNT); 
#endif
    LOGD(TAG "prepare_sw_info_phoneSn_%s\n", Phone_SN);
    if(Phone_SN[0] != 0){
        print_verinfo(info, &len,  "PhoneSN", Phone_SN);
    }



	if (GSM_bt_cal_pass)
	{
		sprintf(val_GSM_BT,"PASS");
	}
	else
	{
		sprintf(val_GSM_BT,"FAIL");
	}

    print_verinfo(info, &len,  uistr_gsm_cali_info, val_GSM_BT);	

	#ifdef TD_WCDMA_CALIBRATION_SEPARATE//WT93950 demand separate test
	if (TD_bt_cal_pass)
	{
		sprintf(val_TD_WCDMA_BT,"PASS");
	}
	else
	{
		sprintf(val_TD_WCDMA_BT,"FAIL");
	}
    print_verinfo(info, &len,  uistr_td_cali_info, val_TD_WCDMA_BT);	

	if (WCDMA_bt_cal_pass)
	{
		sprintf(val_TD_WCDMA_BT,"PASS");
	}
	else
	{
		sprintf(val_TD_WCDMA_BT,"FAIL");
	}
    print_verinfo(info, &len,  uistr_wcdma_cali_info, val_TD_WCDMA_BT);	

	#else
	if (TD_bt_cal_pass || WCDMA_bt_cal_pass)
	{
		sprintf(val_TD_WCDMA_BT,"PASS");
	}
	else
	{
		sprintf(val_TD_WCDMA_BT,"FAIL");
	}
	print_verinfo(info, &len,  uistr_td_wcdma_cali_info, val_TD_WCDMA_BT);
	#endif


#ifdef HTC_CID // sunxuewen added
	print_verinfo(info, &len,  "= ", "= : = : = : = : = : = : =");
	char current_cid[9] = {0};
	get_current_cid(current_cid);

	char *cid = current_cid;
	int cid_item = 0;
	for(i = 0; i < sizeof(CID_TABLE)/sizeof(CID_TABLE[0]); i++)
	{
		if(0 == strcmp(CID_TABLE[i].htc_cid_value, current_cid))
		{
			cid_item = CID_TABLE[i].htc_cid_item;
			break;
		}
	}
	cid = get_item_name(ftm_htc_cid_select_items, cid_item);
	if(NULL == cid)
	{
		cid = current_cid;
	}
	print_verinfo(info, &len,  "Current CID", cid);

	
	char nv_cid[9] = {0};
	cid_read_nv(nv_cid);
	cid = nv_cid;
	printf("nv-cid is[%s]\n", nv_cid);
	cid_item = 0;
	for(i = 0; i < sizeof(CID_TABLE)/sizeof(CID_TABLE[0]); i++)
	{
		if(0 == strcmp(CID_TABLE[i].htc_cid_value, nv_cid))
		{
			cid_item = CID_TABLE[i].htc_cid_item;
			break;
		}
	}
	printf("cid-item is[%d]\n", cid_item);
	cid = get_item_name(ftm_htc_cid_select_items, cid_item);
	printf("cid is[%s]\n", cid);
	if(NULL == cid)
	{
		cid = nv_cid;
	}
	print_verinfo(info, &len,  "New     CID", cid);
	printf("success \n");
#endif
	
    return 0;
}

int get_SoftWareinfo(char *info, int size)
{
	create_SoftWareinfo(info, size);

	return 0;
}

bool get_wtVersion_is_ready(void)
{
#ifdef TD_WCDMA_CALIBRATION_SEPARATE//WT93950 demand separate test
	if(GSM_bt_cal_pass && TD_bt_cal_pass && WCDMA_bt_cal_pass)
#else
	if(GSM_bt_cal_pass && (TD_bt_cal_pass || WCDMA_bt_cal_pass))
#endif		
	{
		return true;
	}
	else
	{
		return false;
	}
}

//zhaoqf_sh add for full test :KEY to Fail
int get_back_fail_key(void)
{
#ifdef WT_AUTOTEST_FAIL_KEY_VOLUMEUP
	return KEY_VOLUMEUP;
#else
	return KEY_POWER;
#endif
}

#define HARDWARE_MAX_ITEM_LONGTH		64
#define HW_INFO_DEVICE "/dev/hardwareinfo"
#define HARDWARE_ID						'H'
#define HARDWARE_LCD_GET				_IOWR(HARDWARE_ID, 0x01, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_TP_GET					_IOWR(HARDWARE_ID, 0x02, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_FLASH_GET				_IOWR(HARDWARE_ID, 0x03, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_FRONT_CAM_GET			_IOWR(HARDWARE_ID, 0x04, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_BACK_CAM_GET			_IOWR(HARDWARE_ID, 0x05, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_ACCELEROMETER_GET		_IOWR(HARDWARE_ID, 0x06, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_ALSPS_GET			    _IOWR(HARDWARE_ID, 0x07, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_GYROSCOPE_GET			_IOWR(HARDWARE_ID, 0x08, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_MAGNETOMETER_GET		_IOWR(HARDWARE_ID, 0x09, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_BT_GET					_IOWR(HARDWARE_ID, 0x10, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_WIFI_GET				_IOWR(HARDWARE_ID, 0x11, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_GPS_GET			    _IOWR(HARDWARE_ID, 0x12, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_FM_GET			        _IOWR(HARDWARE_ID, 0x13, char[HARDWARE_MAX_ITEM_LONGTH])
#define HARDWARE_BAT_GET				_IOWR(HARDWARE_ID, 0x15, char[HARDWARE_MAX_ITEM_LONGTH])

extern int camera_lowlevel_init(void);

#if defined(FEATURE_FTM_EMMC) && defined(FEATURE_FTM_ROM_CAPACITY)
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <dirent.h>
#include "include/linux/mmc/sd_misc.h"
#include "cust_mcard.h"
#include "mounts.h"
#include "make_ext4fs.h"

#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (0)
#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

#define EXT_CSD_REV                     192 /* R */
#define EXT_CSD_SEC_CNT                 212 /* RO, 4 bytes */
#define EXT_CSD_BOOT_SIZE_MULTI	226
#define EXT_CSD_STRUCTURE	194	/* RO */

struct eMMC {
    int          id;
	char         sys_path[512];
    char         info[1024];
	u32		 	csd[4];
	unsigned char 		ext_csd[512];
	unsigned int capacity;
    bool         avail;
};

#define DEVICE_PATH "/dev/misc-sd"

#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})

#define mod_to_ram(p)  (struct eMMC*)((char*)(p))

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)

static bool eMMC_avail(struct eMMC *mc)
{
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;

    if (mc->id < MIN_SDCARD_IDX || mc->id > MAX_SDCARD_IDX)
        return false;

    sprintf(name, "mmc%d", mc->id - MIN_SDCARD_IDX);

    ptr  = &(mc->sys_path[0]);
    ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(mc->sys_path)))
        goto error;

    while (NULL != (dirp = readdir(dp))) {
        if (strstr(dirp->d_name, name)) {
            ptr += sprintf(ptr, "/%s", dirp->d_name);
            break;
        }
    }

    closedir(dp);

    if (!dirp)
        goto error;

    return true;

error:
    return false;
}

static void eMMC_update_info(struct eMMC *mc, char *info)
{
    struct statfs stat;
	unsigned int e, m;
    char *ptr;
    int rc;
    bool old_avail = mc->avail;
	int inode = 0;
	int result = 0;
	int rev = 0;
	unsigned int sectors = 0;
	unsigned char structure = 0;
	struct msdc_ioctl l_ioctl_arg;
	memset(&l_ioctl_arg,0,sizeof(struct msdc_ioctl));
	l_ioctl_arg.host_num = mc->id;
	l_ioctl_arg.opcode = MSDC_GET_CSD;
	l_ioctl_arg.total_size = 16;
	l_ioctl_arg.buffer = mc->csd;

    mc->avail = eMMC_avail(mc);
	inode = open(DEVICE_PATH, O_RDONLY);
	if (inode < 0) {
		printf("open device error!\n");
		return;
	}
	result = ioctl(inode, MSDC_GET_CSD, &l_ioctl_arg);
	if(result < 0){
		printf("ioct error!\n");
		close(inode);
		return;
		}
	close(inode);
	e = UNSTUFF_BITS(l_ioctl_arg.buffer, 47, 3);
	m = UNSTUFF_BITS(l_ioctl_arg.buffer, 62, 12);
	
	mc->capacity = (1 + m) << (e + 2);
	structure = UNSTUFF_BITS(l_ioctl_arg.buffer, 126, 2);

	memset(&l_ioctl_arg,0,sizeof(struct msdc_ioctl));
	l_ioctl_arg.host_num = mc->id;
	l_ioctl_arg.opcode = MSDC_GET_EXCSD;
	l_ioctl_arg.total_size = 512;
	l_ioctl_arg.buffer = mc->ext_csd;
	
	inode = open(DEVICE_PATH, O_RDONLY);
	if (inode < 0) {
		printf("open device error!\n");
		return;
	}
	result = ioctl(inode, MSDC_GET_EXCSD, &l_ioctl_arg);
	if(result < 0){
		LOGE(TAG "ioct error!\n", __FUNCTION__);
		close(inode);
		return;
		}
	close(inode);	
	
	if (structure == 3) {
		int ext_csd_struct = mc->ext_csd[EXT_CSD_STRUCTURE];
		if (ext_csd_struct > 2) {
			LOGE(TAG " unrecognised EXT_CSD structure version %d\n",ext_csd_struct);
			return;
		}
	}
	rev =mc->ext_csd[EXT_CSD_REV];
	if (rev > 7) {
		LOGE(TAG " unrecognised EXT_CSD revision %d\n",rev);
		return;
	}
	if (rev >= 2) {
		sectors =
			mc->ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
			mc->ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
	}
	mc->capacity = (float)(sectors /1024 * 512)> (mc->capacity)?(float)(sectors /1024 * 512) :(mc->capacity);
	
	ptr  = info;
    ptr += sprintf(ptr, "TotalSize: %.2f GB\n", (float)(mc->capacity)/(1024*1024));
    return;
}

#endif
static int create_hardwareinfo(char *info, int size)
{
    char temp_info[HARDWARE_MAX_ITEM_LONGTH] = "Unknown";
	int len = 0;
	
	g_nr_lines = 0;
	
	int fd = -1;
	
	fd = open(HW_INFO_DEVICE, O_RDWR);
	
	if(fd < 0)
	{
		LOGE(TAG "Fail to open device uart modem\n");
		
		print_verinfo(info, &len,  "Flash", temp_info);	
		print_verinfo(info, &len,  "LCD", temp_info);
		print_verinfo(info, &len,  "TP", temp_info);
		print_verinfo(info, &len,  "Camera", temp_info);
		#ifdef CUSTOM_KERNEL_ACCELEROMETER
		print_verinfo(info, &len,  "Accleerometer", temp_info);
		#endif
		#ifdef CUSTOM_KERNEL_ALSPS
		print_verinfo(info, &len,  "Alsps", temp_info);
		#endif
		#ifdef CUSTOM_KERNEL_GYROSCOPE
		print_verinfo(info, &len,  "Gyrocsop", temp_info);
		#endif
		#ifdef CUSTOM_KERNEL_MAGNETOMETER
		print_verinfo(info, &len,  "Magnetometer", temp_info);
		#endif
		#ifdef MTK_WLAN_SUPPORT
		print_verinfo(info, &len,  "WIFI", temp_info);
		#endif
		#ifdef MTK_BT_SUPPORT
		print_verinfo(info, &len,  "BT", temp_info);
		#endif
		#ifdef MTK_FM_SUPPORT
		print_verinfo(info, &len,  "FM", temp_info);
		#endif
		#ifdef MTK_GPS_SUPPORT
		print_verinfo(info, &len,  "GPS", temp_info);
		#endif
		#ifdef WT_BQ2022A_BAT_ID_SUPPORT
		print_verinfo(info, &len,  "Battery", temp_info);
		#endif
		LOGE(TAG "Fail to open device uart modem end\n");
		return 0;
	}
	else
	{		
		camera_lowlevel_init();
		
		//flash info
		if (ioctl(fd, HARDWARE_FLASH_GET, temp_info)) {
			LOGE(TAG "Get Flash info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Flash", temp_info);
		
		//lcd info
		if (ioctl(fd, HARDWARE_LCD_GET, temp_info)) {
			LOGE(TAG "Get LCD info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "LCD", temp_info);
		
		//tp info
		if (ioctl(fd, HARDWARE_TP_GET, temp_info)) {
			LOGE(TAG "Get TP info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "TP", temp_info);

		#ifdef FEATURE_FTM_MAIN_CAMERA
		//camera info
		if (ioctl(fd, HARDWARE_BACK_CAM_GET, temp_info)) {
			LOGE(TAG "Get Camera_Main info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Camera_Main", temp_info);
		#endif

		#ifdef FEATURE_FTM_SUB_CAMERA
		if (ioctl(fd, HARDWARE_FRONT_CAM_GET, temp_info)) {
			LOGE(TAG "Get Camera_Sub info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Camera_Sub", temp_info);
		#endif

		#ifdef CUSTOM_KERNEL_ACCELEROMETER
		//Accleerometer info
		if (ioctl(fd, HARDWARE_ACCELEROMETER_GET, temp_info)) {
			LOGE(TAG "Get Accleerometer info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Accleerometer", temp_info);
		#endif

		#ifdef CUSTOM_KERNEL_ALSPS
		//Alsps info
		if (ioctl(fd, HARDWARE_ALSPS_GET, temp_info)) {
			LOGE(TAG "Get Alsps info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Alsps", temp_info);
		#endif

		#ifdef CUSTOM_KERNEL_GYROSCOPE
		//Gyrocsop info
		if (ioctl(fd, HARDWARE_GYROSCOPE_GET, temp_info)) {
			LOGE(TAG "Get Gyrocsop info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Gyrocsop", temp_info);
		#endif

		#ifdef CUSTOM_KERNEL_MAGNETOMETER
		//Magnetometer info
		if (ioctl(fd, HARDWARE_MAGNETOMETER_GET, temp_info)) {
			LOGE(TAG "Get Magnetometer info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Magnetometer", temp_info);	
		#endif

		#ifdef MTK_WLAN_SUPPORT
		//wifi info
		if (ioctl(fd, HARDWARE_WIFI_GET, temp_info)) {
			LOGE(TAG "Get WIFI info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "WIFI", temp_info);
		#endif

		#ifdef MTK_BT_SUPPORT
		//bt info
		if (ioctl(fd, HARDWARE_BT_GET, temp_info)) {
			LOGE(TAG "Get BT info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "BT", temp_info);
		#endif

		#ifdef MTK_FM_SUPPORT
		//fm info
		if (ioctl(fd, HARDWARE_FM_GET, temp_info)) {
			LOGE(TAG "Get FM info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
        print_verinfo(info, &len,  "FM", temp_info);
		#endif

		#ifdef MTK_GPS_SUPPORT
		//gps info
		if (ioctl(fd, HARDWARE_GPS_GET, temp_info)) {
			LOGE(TAG "Get GPS info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "GPS", temp_info);
		#endif
        
		#ifdef WT_BQ2022A_BAT_ID_SUPPORT
		//Battery info
		if (ioctl(fd, HARDWARE_BAT_GET, temp_info)) {
			LOGE(TAG "Get BAT info fail: %s", strerror(errno));
			strcpy(temp_info, "Fail");
		}
		print_verinfo(info, &len,  "Battery", temp_info);
		#endif		
	}

#ifdef FEATURE_FTM_CPU_INFO_SUPPORT
	{
	const char *cpuInfo = "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq";
	int fo = 0;
	ssize_t s;
	char *ptr = NULL;
	fo = open(cpuInfo, O_RDONLY);
	if(fo < 0) {
		LOGD(TAG "Fail_to_open %s. Terminate\n", cpuInfo);
	}else {
		s = read(fo, (void *)temp_info, 20);
		if (s <= 0) {
			LOGD(TAG "Fail_to_read %s. Terminate\n", cpuInfo);
		} else {
			LOGD(TAG "read_cupInfo_success %s.\n", temp_info);
			ptr = strchr(temp_info, '\n');
			if (ptr != NULL) {
				*ptr = 0;
			}
			if(temp_info[strlen(temp_info)-1] == '\r')
			{
				temp_info[strlen(temp_info)-1] = 0;
			}
			
			print_verinfo(info, &len,  "CPU", temp_info);
		}
		close(fo);
	}
	}
#endif

#ifdef FEATURE_FTM_RAM_CAPACITY
	{
	int fo = 0;
	ssize_t s;
	char *ptr = NULL;	
	const char *memInfo = "/proc/meminfo";
	s = 0;
	fo = open(memInfo, O_RDONLY);
	if(fo < 0) {
		LOGD(TAG "Fail_to_open %s. Terminate\n", memInfo);
	}else {
		s = read(fo, (void *)temp_info, HARDWARE_MAX_ITEM_LONGTH);
		if (s <= 0) {
			LOGD(TAG "Fail_to_read %s. Terminate\n", memInfo);
		} else {
			ptr = strchr(temp_info, '\n');
			if (ptr != NULL) {
				*ptr = 0;
			}
			if(temp_info[strlen(temp_info)-1] == '\r')
			{
				temp_info[strlen(temp_info)-1] = 0;
			}
		
			LOGD(TAG "read_MemInfo_success %s. len =%d\n", temp_info , strlen(temp_info));
			print_verinfo(info, &len,  "RAM", temp_info);
		}
		close(fo);
	}
	}
#endif
#if defined(FEATURE_FTM_EMMC) && defined(FEATURE_FTM_ROM_CAPACITY)
	{
	struct eMMC *mmc = NULL;
	
	mmc  = mod_to_ram(info+len);
	mmc->id = CUST_EMMC_ID;
	mmc->avail = false;
	
	eMMC_update_info(mmc, mmc->info);
	LOGE("emmc_ram_info_is %s" , mmc->info);
	if(mmc->info != NULL)
	{
		print_verinfo(info, &len,  "ROM", mmc->info);
	}
	else
	{
		print_verinfo(info, &len,  "ROM", "error");
	}
	}
#endif
	
	return 0;
}

static void *background_test(void *priv)
{
    int i = 0;
    item_t *items;
    struct ftm_module *mod;
    struct ftm_param param;
    items = get_item_list();
    while (items[i].name) {
        mod = ftm_get_module(items[i].id);
        if (mod && (BACKGROUND_TEST==mod->test_type) && (mod->needbackRun)) {
            param.name = items[i].name;
            ftm_entry(items[i].id, &param);
            int ret = report_writeByID(mod->id, mod->test_result);
            	LOGD(TAG "%s insert ret =%d\n", __FUNCTION__, ret);
        }      
		i++;
    }
	while(BT_is_testing || wifi_is_testing || GPS_is_testing) sleep(1);

    initbackrunparam();
	return NULL;
}

static void *background_test_timeout(void *priv)
{
	int time_bt = 0;
	int time_wifi = 0;
	int time_sim = 0;
	int bt_stop = 0;
	int wifi_stop = 0;
	
#if defined(FEATURE_FTM_BT) && defined(FEATURE_FTM_WIFI)
	while(!wifi_stop)
#elif defined(FEATURE_FTM_WIFI)
	while(!wifi_stop)
#elif defined(FEATURE_FTM_BT)
	while(!bt_stop)
#else
	return NULL;
#endif
	{
		#ifdef FEATURE_FTM_BT
		if(BT_is_testing)
		{
			time_bt++;
		}
        
        LOGD(TAG "full_test_mode_bt_time: %d, bt_testing: %d", time_bt, BT_is_testing);
		if((time_bt>25) && !bt_test_pass){
			bt_test_pass = true;//stop bt test thread
			BT_test_canceled = true;
			bt_stop = 1;
		}
		#endif

		#ifdef FEATURE_FTM_WIFI	
		if(wifi_is_testing)
		{
			time_wifi++;
		}
        
        LOGD(TAG "full_test_mode_wifi_time: %d, wifi_testing: %d", time_wifi, wifi_is_testing);
		if((time_wifi>20) && !wifi_test_pass){
			wifi_test_pass = true;//stop wifi test thread
			wifi_test_canceled = true;
			wifi_stop = 1;
		}
		#endif

		#ifdef FEATURE_FTM_BT
		if(bt_test_pass || BT_test_canceled) bt_stop = 1;
		#endif
		
		#ifdef FEATURE_FTM_WIFI
		if(wifi_test_pass || wifi_test_canceled) wifi_stop = 1;
		#endif
		
		sleep(1);
	}
	return NULL;
}

#ifdef FEATURE_FTM_GPS	
static void *background_gps_timeout(void *priv)
{
	int time_gps_ttff = 0;
	int gps_stop = 0;
	
	while(!gps_stop){
		if(GPS_is_testing)
		{
			time_gps_ttff++;
		}
        
        LOGD(TAG "full_test_mode_GPS_time: %d, wifi_testing: %d", time_gps_ttff, GPS_is_testing);
		if((time_gps_ttff>40) && !gps_test_pass){
			gps_test_pass = true;//stop gps test thread
			gps_stop = 1;
		}
                        
		if(gps_test_pass) gps_stop = 1;
		sleep(1);
	}
	return NULL;
}
#endif

static void *init_full_test_new_mode(void *priv)
{
	pthread_t background_test_thd; 
	pthread_t background_test_timeout_thd; 
    pthread_t background_gps_timeout_thd;

	sleep(1);
	pthread_create(&background_test_timeout_thd,NULL,background_test_timeout,NULL);
#ifdef FEATURE_FTM_GPS	
	sleep(1);
	pthread_create(&background_gps_timeout_thd,NULL,background_gps_timeout,NULL);
#endif
	sleep(1);
	pthread_create(&background_test_thd,NULL,background_test,NULL);
	pthread_join(background_test_thd,NULL);
	pthread_join(background_test_timeout_thd,NULL);
#ifdef FEATURE_FTM_GPS		
	pthread_join(background_gps_timeout_thd,NULL); 
#endif
	background_test_complete = true;
	return NULL;
}

/***************************Add by zhaoqf_sh End************************************/
static int full_test_mode(char *buf, int bufsz)
{
    int i = 0;
    item_t *items;
    struct ftm_module *mod;
    struct ftm_param param;
    //handle of testreport.log
    FILE *fp = NULL;
    int stopmode = 0;
    char *stopprop = ftm_get_prop("FTM.FailStop");
    pthread_t full_test_thd; //zhaoqf_sh

    if (stopprop && !strncasecmp(stopprop, "yes", strlen("yes")))
        stopmode = 1;

    LOGD(TAG "full_test_mode: %d", stopmode);

//    items = get_manual_item_list();

    //items = ftm_cust_items;
    items = get_item_list();
    initbackrunparam();
    is_full_mode = 1;
    pthread_create(&full_test_thd,NULL,init_full_test_new_mode,NULL);

	LOGD(TAG "get_manual_item_list end");

    while (items[i].name)
	{
		LOGD(TAG "name = %s,id = %d,mode=%d",items[i].name,items[i].id,items[i].mode);
        //if(items[i].mode != FTM_AUTO_ITEM)
		{
			LOGD(TAG "%s:%d", items[i].name, items[i].id);

			switch (items[i].id)
			{
			case ITEM_IDLE: /* skip items */
				break;
			case ITEM_REPORT:
				item_test_report(items, buf, bufsz);
				break;
			default:
				mod = ftm_get_module(items[i].id);
				if (mod && mod->visible)
				{
                        //zhaoqf_sh add for full test 
                        if(is_full_mode && ((mod->test_type && (true == mod->needbackRun)) || (true == mod->needSkip)) && (mod->id != ITEM_GPS)) 
                        {
                            i++;
                            continue;
                        }
					param.name = items[i].name;
					ftm_entry(items[i].id, &param);
					if (stopmode && mod->test_result != FTM_TEST_PASS)
						continue;
                        int ret = report_writeByID(mod->id, mod->test_result);
                        if(-1== ret){
                        	LOGD(TAG "%s insert ret =%d\n", __FUNCTION__, ret);
                        }
                    }            
				break;
			}
		}
		i++;
	}
	if(!background_test_complete) ui_printf("Waiting for background test complete ...\n");
	pthread_join(full_test_thd,NULL);
	background_test_complete = false;
	is_full_mode = 0;
	item_test_report(get_item_list(), buf, BUFSZ);
    return 0;
}

static int auto_test_mode(char *buf, int bufsz)
{
    int i = 0;
    item_t *items, *cust_items;
    struct ftm_module *mod;
    struct ftm_param param;
    //handle of testreport.log
    FILE *fp = NULL;
    int stopmode = 0;
    char *stopprop = ftm_get_prop("FTM.FailStop");

    if (stopprop && !strncasecmp(stopprop, "yes", strlen("yes")))
        stopmode = 1;

    LOGD(TAG "auto_test_mode: %d", stopmode);

    items = get_auto_item_list();

    while (items[i].name) {
        LOGD(TAG "%s:%d", items[i].name, items[i].id);
        switch (items[i].id) {
        case ITEM_IDLE: /* skip items */
            break;
        case ITEM_REPORT:
            item_test_report(items, buf, bufsz);
            break;
        default:
            mod = ftm_get_module(items[i].id);
            if (mod && mod->visible) {
                param.name = items[i].name;
                ftm_entry(items[i].id, &param);
                if (stopmode && mod->test_result != FTM_TEST_PASS)
                    continue;
            }
            break;
        }
        i++;
    }

    return 0;
}

static int item_test_mode(char *buf, int bufsz)
{
    int chosen_item = 0;
    bool exit = false;
    struct itemview itv;  /* item test menu */
    struct ftm_param param;
    struct ftm_module *mod = NULL; 
    text_t  title;
    item_t *items;

    LOGD(TAG "item_test_mode");

    is_full_mode = 0;
    items = get_item_list();

    ui_init_itemview(&itv);
    init_text(&title, uistr_item_test, COLOR_YELLOW);

    itv.set_title(&itv, &title);
    itv.set_items(&itv, items, 0);

    while (1) {    
        chosen_item = itv.run(&itv, &exit);
        if (exit == true) 
            break;
        switch (chosen_item) {
        case ITEM_REPORT:
            item_test_report(items, buf, bufsz);
            break;
        default:
            param.name = get_item_name(items, chosen_item);
            ftm_entry(chosen_item, &param);
            /*insert sqlite*/
            mod = ftm_get_module(chosen_item);
            int ret = report_writeByID(mod->id, mod->test_result);
            if(-1== ret){
            	LOGD(TAG "%s insert ret =%d\n", __FUNCTION__, ret);
            }
            break;
        }
    }
    return 0;
}

char testmode[255]= {0};

void inittestmodevalue(void)
{
    memset(testmode, 0, sizeof(testmode));
    if(g_pcba_test == false){
        sIsBoardTest = isBoardTestSuccess(false);
        if(sIsBoardTest){
            sprintf(testmode, "MMI test: PASS\n");
        }
        else{
            sprintf(testmode, "MMI test: FAIL\n");
        }
    }else{
        s_pcba_test_success = isBoardTestSuccess(true);
        if(s_pcba_test_success){
            sprintf(testmode, "PCBA test: PASS\n");
        }
        else{
            sprintf(testmode, "PCBA test: FAIL\n");
        }
    }
    LOGD(TAG "%s testmode ret =%s\n", __FUNCTION__, testmode);
}

static int test_mode_menu(char* buf, int bufsz)
{
    int chosen_item = 0;
    bool exit = false;
    struct itemview itv;  /* item test menu */
    text_t  title;
    text_t testmode_info; //zhaoqf_sh
    
    ui_init_itemview(&itv);
    inittestmodevalue();
    if(g_pcba_test == true){
        init_text(&title, uistr_pcba_test, COLOR_YELLOW);
        itv.set_items(&itv, ftm_menu_items_pcba, 0);
    }else{
        init_text(&title, uistr_mmi_test, COLOR_YELLOW);
        itv.set_items(&itv, ftm_menu_items_mmi, 0);
    }
    itv.set_title(&itv, &title);
    init_text(&testmode_info, testmode, COLOR_YELLOW);
    itv.set_text(&itv,&testmode_info);//zhaoqf_sh   
    while (1) {    
        chosen_item = itv.run(&itv, &exit);
        if (exit == true) 
            break;
        switch (chosen_item) {
            case ITEM_FULL_TEST:
            case ITEM_PCBA_FULL_TEST:
                full_test_mode(buf, BUFSZ);
                break;
            case ITEM_ITEM_TEST:
            case ITEM_PCBA_ITEM_TEST:
                item_test_mode(buf, BUFSZ);
                break;
            case ITEM_REPORT:
                item_test_report(get_item_list(), buf, BUFSZ);
                break;
            default:
                break;
        }
    }
    return 0;
}

// req:htc_cid select  sunxuewen begin
static int ftm_htc_cid_select_mode(char *buf, int bufsz)
{
	int chosen_item = 0;
	bool exit = false;
	struct itemview itv;
	text_t  title;
	item_t *items;

	LOGD(TAG "ftm_htc_cid_select_mode");

	is_full_mode = 0;
	items = ftm_htc_cid_select_items;

	ui_init_itemview(&itv);
	init_text(&title, uistr_htc_cid_select, COLOR_WHITE);

	itv.set_title(&itv, &title);
	itv.set_items(&itv, items, 0);

	while (1) 
	{    
		chosen_item = itv.run(&itv, &exit);
		if (exit == true) 
		{
			break;
		}

		switch(chosen_item)
		{
			default:
			{
				char *cid;
				int i = 0;
				for(i = 0; i < sizeof(CID_TABLE)/sizeof(CID_TABLE[0]); i++)
				{
					if(CID_TABLE[i].htc_cid_item == chosen_item)
					{
						cid = CID_TABLE[i].htc_cid_value;
						break;
					}
				}

				cid_draw_text(cid_modify(cid), ftm_htc_cid_select_items[i].name);
			}
			break;
			case ITEM_HTC_CID_DEVICE_INFO:
				cid_draw_device_info();
			break;
		}



	}
	return 0;
}
// req:htc_cid select  sunxuewen end


static int debug_test_mode(char *buf, int bufsz)
{
    int chosen_item = 0;
    bool exit = false;
    struct itemview itv;  /* item test menu */
    struct ftm_param param;
    text_t  title;
    item_t *items;

    LOGD(TAG "debug_test_mode");

    items = get_debug_item_list();

    ui_init_itemview(&itv);
    init_text(&title, uistr_item_test, COLOR_YELLOW);

    itv.set_title(&itv, &title);
    itv.set_items(&itv, items, 0);

    while (1) {
        chosen_item = itv.run(&itv, &exit);
        if (exit == true)
            break;
        switch (chosen_item) {
        default:
			LOGD(TAG "chosen_item=%d",chosen_item);
            param.name = get_item_name(items, chosen_item);
            ftm_debug_entry(chosen_item, &param);
            break;
        }
    }
    return 0;
}

#ifdef FEATURE_FTM_PMIC_632X
extern int battery_fg_current;
#endif

static int test_module()
{

		int arg = 0;
		int id = -1;
		int write_len = 0;
		struct ftm_param param;
		char test_result[128] = {0};
		item_t *items;
		struct ftm_module *mod;
		char *prop_name = NULL;
		char *prop_val = NULL;
		char result[3][16] = {"not test\r\n", "pass\r\n", "fail\r\n" };
		char temp_at_command[128] = {0};
		int i = 0;
		#if 0//zhaoqf_sh modify
		int at_command_len = 0;
		char p[16] = {0};
		items = get_item_list();
		#endif
		char bufbattdata[10] = {0};
		char *p;
		items = pc_control_items;
		p = at_command;
		at_command[4] = 0;
		
		char buf[8] = {0};
		strcpy(buf, "quit");
		strcpy(bufbattdata, "battdata");

		while(1)
		{
			if(strlen(at_command) <= 3)
			{
				continue;
			}
			LOGD("The length of at_command is %d, at_command=%s\n", strlen(at_command), at_command);
			
            if(!memcmp(buf, at_command, 4))
            {
                memset(at_command, 0, sizeof(at_command));
                break;
            }

			if(!memcmp(bufbattdata, at_command, 8))
            {
				need_get_batt_data = 1;				
            	write_len = write(usb_com_port, "ok\r\n", sizeof("ok\r\n"));
				memset(at_command, 0, sizeof(at_command));
                continue;
			}

		#if 0//zhaoqf_sh modify
		    pthread_mutex_lock (&at_command_mutex);
            at_command_len = strlen(at_command);
            strcpy(p, at_command);
            memset(at_command, 0, sizeof(at_command));
			if(at_command_len <= 3){
		    pthread_mutex_unlock (&at_command_mutex);
				continue;
            }

            if(!memcmp(buf, p, 4))
            {
		    pthread_mutex_unlock (&at_command_mutex);
//                memset(at_command, 0, sizeof(at_command));
                break;
            }
			pthread_mutex_unlock (&at_command_mutex);
			LOGD(TAG "at command:%d, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, %s,%d \n",
				strlen(at_command), at_command[0], at_command[1], at_command[2], at_command[3],
				buf[0], buf[1], buf[2], buf[3], at_command, strcmp(buf, at_command));			

			id = get_item_id(pc_control_items, p);			
		#endif
			id = get_item_id(pc_control_items, at_command);	
			LOGD(TAG "test item id is %d\n", id);
			if(id >= 0)
			{
				LOGD(TAG "before get_item_name");
				param.name = get_item_name(items, id);
				if(param.name == NULL)
				{
					param.name = get_item_name(ftm_menu_items, id);
				}
				LOGD(TAG "after get_item_name is %s\n", param.name);
				if(param.name != NULL)
				{
					mod = ftm_get_module(id);
					if (mod && mod->visible)
					{
						int ret = 0;
						ret = ftm_entry(id, &param);
                        LOGD(TAG "ftm_module_test_result:%d , ret : %d\n", mod->test_result, ret);

						if(ret == -EINVAL)//can't entry function ,return -EINVAL, fail
						{
							strcpy(test_result, result[2]);
						}
						else
						{
                        if(mod->test_result >= FTM_TEST_MAX)
                        {
                            strcpy(test_result, result[0]);
                        }
                        else
                        {
#ifdef FEATURE_FTM_PMIC_632X
							if(mod->id == ITEM_CHARGER)
							{
								char sign[2] = {0, 0};
								memset(test_result, 0, sizeof(test_result));
								if(battery_fg_current>=0)
								{
									sign[0] = '+';
								}
								sprintf(test_result, "%s%s%d", result[mod->test_result], sign, battery_fg_current);
							}
							else
#endif
								strcpy(test_result, result[mod->test_result]);
					}
					}
				}
			}
			}
			else
			{
				strcpy(test_result, "Cannot find the module!\r\n");
			}
            LOGD(TAG "before write data to pc\n");
            while(usb_status != 1)
            {
                sleep(1);
            }
			
#ifdef FEATURE_FTM_AUDIO_AUTO_TEST_SUPPORT
			if(1 != gAudio_auto_test)
#endif
			{
			write_len = write(usb_com_port, test_result, strlen(test_result));
			LOGD(TAG "after write data to pc\n");
			if(write_len != (int)strlen(test_result))
			{
				LOGD(TAG "write data to pc fail\n");
			}
			}
			memset(at_command, 0, sizeof(at_command));
			
		}
		LOGD(TAG "test_result is %s, the %s\n", test_result, test_data);
		return 0;
}

static int is_pc_control(int fd)
{
    struct timeval startime, endtime;
	double time_use = 0;
	int read_from_usb = 0;
	char USB_read_buffer[BUFSZ] = {0};
	bool pc_control = false;
#ifdef ENTER_TP_TEST
	double max_time = 100000;
#else
	double max_time = 1500000;
#endif	
	gettimeofday(&startime, 0);
	LOGD(TAG "time_use = %lf\n", time_use);

	if(usb_com_port == -1)
	{
		return 0;
	}

	while(time_use < max_time)
	{
//		LOGD(TAG "time_use = %lf\n", time_use);
        if(usb_com_port != -1)
        {
            read_from_usb = read(usb_com_port, USB_read_buffer, sizeof(USB_read_buffer));
//			LOGD(TAG "read_from_usb = %d, USB_read_buffer = %s\n", fd, USB_read_buffer);
		}
		if(read_from_usb == -1)
		{
			gettimeofday(&endtime, 0);
			time_use = 1000000 * (endtime.tv_sec - startime.tv_sec) +
				endtime.tv_usec - startime.tv_usec;
            continue;
		}
		else if (read_from_usb > 0)
		{
			if(strncmp(USB_read_buffer, START, strlen(START)) == 0)
			{
                LOGD(TAG "start\n");
                int len = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
				if(len != (int)strlen("pass\r\n"))
				{
					LOGD(TAG "write pass fail in is_pc_control");
				}
				else
				{
					LOGD(TAG "write pass in is_pc_control");
					pc_control = true;
                    usb_status = 1;
					break;
				}
			}
		}
	}
//	write(fd,"ok",strlen("ok"));
	return pc_control;
}


//MTKBEGIN  [mtk0625][DualTalk]
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
#define EXT_MD_IOC_MAGIC			'E'
#define EXT_MD_IOCTL_LET_MD_GO		_IO(EXT_MD_IOC_MAGIC, 1)
#define EXT_MD_IOCTL_REQUEST_RESET	_IO(EXT_MD_IOC_MAGIC, 2)
#define EXT_MD_IOCTL_POWER_ON_HOLD	_IO(EXT_MD_IOC_MAGIC, 3)

int boot_modem(int is_reset)
{
    LOGD(TAG "%s\n", __FUNCTION__);

	int ret;
	int ext_md_ctl_n0, ext_md_ctl_n1;

	ext_md_ctl_n0 = open("/dev/ext_md_ctl0", O_RDWR);
	if(ext_md_ctl_n0 <0) {
        LOGD(TAG "open ext_md_ctl0 fail");
		return	ext_md_ctl_n0;
	}
	ret = ioctl(ext_md_ctl_n0, EXT_MD_IOCTL_POWER_ON_HOLD, NULL);
	if (ret < 0) {
        LOGD(TAG "power on modem fail");
		return	ret;
	}

	ext_md_ctl_n1 = open("/dev/ext_md_ctl1", O_RDWR);
	if(ext_md_ctl_n1 <0) {
        LOGD(TAG "open ext_md_ctl_n1 fail");
		return	ext_md_ctl_n1;
	}
	ret = ioctl(ext_md_ctl_n1, EXT_MD_IOCTL_LET_MD_GO, NULL);
	if (ret < 0) {
        LOGD(TAG "EXT_MD_IOCTL_LET_MD_GO fail");
		return	ret;
	}

	return	ret;
}
#endif  /* MTK_DT_SUPPORT */
//MTKEND    [mtk80625][DualTalk]

void *read_data_thread_callback(void* data)
{
	int read_from_usb = 0;
	char USB_read_buffer[BUFSZ] = {0};
	bool exit = false;

	while(1)
    {
        if(usb_plug_in == 0)
        {
            LOGD("FACTORY.C usb_plug_in == 0\n");
            continue;
        }
        else if(is_USB_State_PlugIn())
        {
            open_usb();
        }
		if(usb_com_port != -1)
    	{
            read_from_usb = read_a_line_test(usb_com_port, USB_read_buffer, sizeof(USB_read_buffer));
		}
        else
    	{
            continue;
		}
		if(read_from_usb <= 0) //== -1) //zhaooqf_sh modify
		{
            continue;
		}
		else if(read_from_usb > 0)
		{
            LOGD(TAG "read from usb is %s\n",USB_read_buffer);
			if(read_from_usb > 3)
			{
			USB_read_buffer[read_from_usb-1] = 0;
            pthread_mutex_lock (&at_command_mutex);
			LOGD(TAG "-----------> AT COMMAND = %s\n", at_command);
            pthread_mutex_unlock (&at_command_mutex);

		    if(strncmp(USB_read_buffer, STOP, strlen(STOP)) == 0){
                LOGD(TAG "stop\n");
                pthread_mutex_lock (&at_command_mutex);
				strcpy(at_command, "quit");
				LOGD(TAG "compare at_command and quit:%d\n", strncmp(at_command, "quit", strlen("quit")));
                pthread_mutex_unlock (&at_command_mutex);
                int n = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
				if(n != (int)strlen("pass\r\n"))
			    {
				    LOGD(TAG "Write stop pass fail\n");
			    }
				else
				{
					LOGD(TAG "Write stop pass successfully\n");
				}
				close(usb_com_port);
				break;
		    }
			else if(strncmp(USB_read_buffer, START, strlen(START)) == 0)
			{
                LOGD(TAG "start\n");
                int n = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
                usb_status = 1;
				if(n != (int)strlen("pass\r\n"))
			    {
				    LOGD(TAG "Write start pass fail\n");
			    }
				else
				{
					 LOGD(TAG "Write start pass successfully\n");
				}
			}
			else if(strncmp(USB_read_buffer, REQUEST_DATA, strlen(REQUEST_DATA)) == 0)
			{

				LOGD(TAG "name:%s, mac:%s, rssi:%d, channel:%d ,rate%d\n", return_data.wifi.wifi_name,
					return_data.wifi.wifi_mac, return_data.wifi.wifi_rssi,
					return_data.wifi.channel, return_data.wifi.rate);
				int i = 0;
				for(i = 0 ; i < return_data.bt.num; i++)
				{
				LOGD(TAG "bt_mac:%s, rssi:%d\n", return_data.bt.bt[i].bt_mac, return_data.bt.bt[i].bt_rssi);
				}
				char temp_buf[2048] = {0};
				memcpy(temp_buf, &return_data, sizeof(return_data));
				strcpy(temp_buf+sizeof(return_data), "\r\n");
			    int n = write(usb_com_port, temp_buf, sizeof(temp_buf));
			   // int n = write(fd, return_data.wifi.wifi_name, sizeof(return_data.wifi.wifi_name));
			    if(n != sizeof(temp_buf))
			    {
				    LOGD(TAG "Write test_data fail,%d\n",  sizeof(temp_buf));
			    }
				else
				{
					 LOGD(TAG "Write test_data successfully,%d\n", sizeof(temp_buf));
				}
			}
			else if(strncmp(USB_read_buffer, VERSION, strlen(VERSION))==0)
			{
				display_version(1);
				int n = write(usb_com_port, "pass\r\n", sizeof("pass\r\n"));
				if(n != sizeof("pass\r\n"))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}
			}
#ifdef WT_FTM_PHONE_MIC_HEADSET_LOOPBACK
			else if(strncmp(USB_read_buffer, PMIC_HEADSET_LOOPBACK_STOP, strlen(PMIC_HEADSET_LOOPBACK_STOP))==0)
			{
				audio_phoneMicToHeadset_loopback_close();
				int n = write(usb_com_port, "stop\r\n", sizeof("stop\r\n"));
				if(n != sizeof("stop\r\n"))
				{
					LOGD(TAG "Write data phoneMicToHeadset_loopback fail\n");
				}
				else
				{
					LOGD(TAG "Write data phoneMicToHeadset_loopback successfully\n");
				}		
			}
#endif
#ifdef WT_FTM_PHONE_REFMIC_HEADSET_LOOPBACK
			else if(strncmp(USB_read_buffer, PREFMIC_HEADSET_LOOPBACK_STOP, strlen(PREFMIC_HEADSET_LOOPBACK_STOP))==0)
			{
				audio_phoneMic2ToHeadset_loopback_close();
				int n = write(usb_com_port, "stop\r\n", sizeof("stop\r\n"));
				if(n != sizeof("stop\r\n"))
				{
					LOGD(TAG "Write data phoneMic2ToHeadset_loopback fail\n");
				}
				else
				{
					LOGD(TAG "Write data phoneMic2ToHeadset_loopback successfully\n");
				}		
			}
#endif
#ifdef WT_FTM_HEADSET_MIC_RECEIVER_LOOPBACK
			else if(strncmp(USB_read_buffer, HMIC_REC_LOOPBACK_STOP, strlen(HMIC_REC_LOOPBACK_STOP))==0)
			{
				audio_HMicToReveiver_loopback_close();
				int n = write(usb_com_port, "stop\r\n", sizeof("stop\r\n"));
				if(n != sizeof("stop\r\n"))
				{
					LOGD(TAG "Write data HMicToReveiver_loopback fail\n");
				}
				else
				{
					LOGD(TAG "Write data HMicToReveiver_loopback successfully\n");
				}
			}
#endif
#ifdef WT_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
			else if(strncmp(USB_read_buffer, HMIC_SPK_LOOPBACK_STOP, strlen(HMIC_SPK_LOOPBACK_STOP))==0)
			{
				audio_HeadsetToSPK_loopback_close();
				int n = write(usb_com_port, "stop\r\n", sizeof("stop\r\n"));
				if(n != sizeof("stop\r\n"))
				{
					LOGD(TAG "Write data HeadsetToSPK_loopback fail\n");
				}
				else
				{
					LOGD(TAG "Write data HeadsetToSPK_loopback successfully\n");
				}
			}
#endif
#ifdef WT_FTM_HEADSET_MIC_EARPHONE_LOOPBACK
			else if(strncmp(USB_read_buffer, HMIC_EARPHONE_LOOPBACK_STOP, strlen(HMIC_EARPHONE_LOOPBACK_STOP))==0)
			{
				audio_HMicToEar_loopback_close();
				int n = write(usb_com_port, "stop\r\n", sizeof("stop\r\n"));
				if(n != sizeof("stop\r\n"))
				{
					LOGD(TAG "Write data HMicToEar_loopback fail\n");
				}
				else
				{
					LOGD(TAG "Write data HMicToEar_loopback successfully\n");
				}
			}
#endif
            else if(strncmp(USB_read_buffer, READ_BARCODE, strlen(READ_BARCODE))==0)
            {
                int ccci1 = -1, i = 0;
                char result[BUFSZ] = {0};
                if(is_support_modem(1))
                {
	                LOGD(TAG "MTK_ENABLE_MD1\n");

	                ccci1= openDevice();
                }
				else if(is_support_modem(2))
				{
					ccci1 = openDeviceWithDeviceName("/dev/ccci2_tty0");
				}

				if(-1 == ccci1) {
					LOGD(TAG "Fail to open CCCI interface\n");
					return 0;
				}
				for (i = 0; i<30; i++) usleep(50000);
				send_at (ccci1, "AT\r\n");
				wait4_ack (ccci1, NULL, 3000);
				
                getBarcode(ccci1,result);
                char *ptr = strchr(result, '\"');
                if ((ptr != NULL) && (strlen(result) > 0)) {
                    *ptr = 0;
                }
                else
                {
                    strcpy(result, "fail\r\n");
                }
                int n = write(usb_com_port, result, sizeof(result));
				if(n != sizeof(result))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}
                if(-1 != ccci1)
                {
                    closeDevice(ccci1);
                }
            }
            else if(strncmp(USB_read_buffer, WRITE_BARCODE, strlen(WRITE_BARCODE))==0)
            {
                LOGD(TAG "Entry write barcode!\n");
                int ccci1 = -1,i = 0;
                int result = -1;
                char *barcode = strchr(USB_read_buffer, '=');
				char return_result[16] = {0};
                if(barcode == NULL)
                {
                    LOGD(TAG "barcode is null!\n");
                }
                else
                {
                    barcode++;
                    LOGD(TAG "%s\n", barcode);
                    if(is_support_modem(1))
                    {
	                    LOGD(TAG "MTK_ENABLE_MD1\n");

	                    ccci1= openDevice();
                    }
					else if(is_support_modem(2))
					{
						ccci1 = openDeviceWithDeviceName("/dev/ccci2_tty0");
					}
					
					if(-1 == ccci1) {
						LOGD(TAG "Fail to open CCCI interface\n");
						return 0;
					}
					for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
					send_at (ccci1, "AT\r\n");
					wait4_ack (ccci1, NULL, 3000);

                    result = write_barcode(ccci1, barcode);
                }
				
				if(result == 0)
				{
					strncpy(return_result, "pass\r\n", strlen("pass\r\n"));
				}
				else
				{
					strncpy(return_result, "fail\r\n", strlen("fail\r\n"));
				}
				int n = write(usb_com_port, return_result, strlen(return_result));

				if(n != strlen(return_result))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}

                if(-1 != ccci1)
                {
                    closeDevice(ccci1);
                }
				
            }
			else
			{
               LOGD(TAG "module\n");
               //test_module(fd, USB_read_buffer);
               int ret = 0;
			   int id = -1;
			   ret = get_AT_command(USB_read_buffer);
			   if(ret == 1)
			   {
					id = get_item_id(pc_control_items, USB_read_buffer);
					switch(id)
					{
						case ITEM_VIBRATOR:
							#ifdef FEATURE_FTM_VIBRATOR
							vibrator_test_exit = true;
							#endif
							break;
						case ITEM_LED:
							#ifdef FEATURE_FTM_LED
							keypadled_test_exit = true;
							led_test_exit = true;
							#endif
							break;
						default:
							break;
					}
			   }
			   else if(ret ==2)
			   {

					int n = write(usb_com_port, "pass\r\n", sizeof("pass\r\n"));
					if(n != sizeof("pass\r\n"))
					{
						LOGD(TAG "Write test_data in version fail\n");
					}
					else
					{
						LOGD(TAG "Write test_data in version successfully\n");
					}

			   }
		    }
				}

			LOGD(TAG "BOOL IS %d\n", exit);
		}
	}//while

	pthread_exit(NULL);
	return NULL;
}



static int pc_control_mode(int fd)
{
	LOGD(TAG "CALL pc_control_mode1");
	test_module();
	return 0;
}

void initboardtest(void)
{
    FILE *fp;
    char capacity[10] = {0};

    memset(boardtest, 0, sizeof(boardtest));
    sIsBoardTest = isBoardTestSuccess(false);
    if(sIsBoardTest){
        sprintf(boardtest+strlen(boardtest), "MMI test: PASS\n");
    }
    else{
        sprintf(boardtest+strlen(boardtest), "MMI test: FAIL\n");
    }
    
    s_pcba_test_success = isBoardTestSuccess(true);
    if(s_pcba_test_success){
        sprintf(boardtest+strlen(boardtest), "PCBA test: PASS\n");
    }
    else{
        sprintf(boardtest+strlen(boardtest), "PCBA test: FAIL\n");
    }

#ifdef FEATURE_BATTERY_LOW_LEVEL
    if (NULL != (fp = fopen(BATT_CAPACITY_FILE, "r"))) {
        LOGD(TAG "%s fp = %d\n", __FUNCTION__, fp);
        if(fgets(capacity, 10, fp)){
            LOGD(TAG "%s capacity = %s\n", __FUNCTION__, capacity);
            if(atoi(capacity) < 20){
                isBattLowLevel = true;
                sprintf(boardtest+strlen(boardtest), "\nBattery level too low %d%s\n", atoi(capacity), "%");
            }else{
                sprintf(boardtest+strlen(boardtest), "\nBattery level is %d%s\n", atoi(capacity), "%");
            }
        }
        fclose(fp);
    }
#endif    
    LOGD(TAG "%s sw_info len =%d\n", __FUNCTION__, strlen(boardtest));
}

int main(int argc, char **argv)
{
	int exit = 0;
	int    sel=0;
	int nr_line=0;
	int avail_lines = 0;
	bool   quit=false;
	char  *buf = NULL;

	//int n;
    struct ftm_param param;
    struct itemview fiv;  /* factory item menu */
    struct itemview miv;  /* mini-ui item menu */
    struct textview hw_info;   /* hardware info */
	//struct textview vi;   /* version info */
    //struct itemview ate;  /* ATE factory mode*/
    item_t *items;
    text_t ftm_title;
    int bootMode;
	int g_fd_atcmd = -1, g_fd_uart = -1;
    int g_hUsbComPort = -1;

	//text_t vi_title;
	//text_t ate_title;
    text_t rbtn;
    text_t info;
    text_t hw_info_tittle;
    text_t software_info; //zhaoqf_sh
	pthread_t read_thread;

    ui_init();

    /* CHECKME! should add this fuctnion to avoid UI not displayed */
 	//ui_print("factory mode\n");
    show_slash_screen(uistr_factory_mode, 1000);

    bootMode = getBootMode();

    if(ATE_FACTORY_BOOT == bootMode)
    {
        ui_print("Enter ATE factory mode...\n");

        ate_signal();

        while(1){}
    }
    else if(FACTORY_BOOT == bootMode)
    {
		buf = malloc(BUFSZ);
		if (NULL == buf)
		{
		    ui_print("Fail to get memory!\n");
		}

		ftm_init();
		
        report_create(get_item_list(), true);
        report_create(get_item_list(), false);

		avail_lines = get_avail_textline();
#if 0 //zhaoqf_sh              
		if (!read_config(FTM_CUST_FILE1))
			read_config(FTM_CUST_FILE2);

		usb_com_port = open("dev/ttyGS0", O_RDWR | O_NOCTTY | O_NDELAY);

        LOGD(TAG "Open USB dev/ttyGS0 %s.\n", (-1==usb_com_port)? "failed":"success");
#else
		if(-1 == COM_Init (&g_fd_atcmd, &g_fd_uart, &g_hUsbComPort))
		{
			LOGE(TAG "COM_Init init fail!\n");
		}
		usb_com_port = g_hUsbComPort;
		LOGD(TAG "Open USB dev_usb_com_port=%d  g_fd_atcmd=%d \n", usb_com_port, g_fd_atcmd);

		if(g_fd_atcmd != -1)
		{
			close(g_fd_atcmd);
			g_fd_atcmd = -1;
		}
#endif

		if(is_pc_control(usb_com_port))
		{
			pthread_create(&read_thread, NULL, read_data_thread_callback, (void *)&usb_com_port);
			LOGD(TAG "after create pthread");
			status = 1;
			pc_control_mode(usb_com_port);
			ALOGD(TAG "pc control stops in if()!\n");
			status = 0;
		}

		LOGD(TAG "pc control stops!\n");

		ui_init_itemview(&fiv);
		ui_init_itemview(&miv);
		//ui_init_textview(&vi, textview_key_handler, &vi);
		ui_init_textview(&hw_info, textview_key_handler, &hw_info);

		init_text(&ftm_title, uistr_factory_mode, COLOR_YELLOW);
		//init_text(&vi_title, uistr_version, COLOR_YELLOW);
		//init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
		//init_text(&info, buf, COLOR_YELLOW);
		init_text(&hw_info_tittle, uistr_hardware_info, COLOR_YELLOW);

	    initboardtest();
	    init_text(&software_info, boardtest, COLOR_WHITE);//zhaoqf_sh
		items = ftm_menu_items;
		fiv.set_title(&fiv, &ftm_title);
		fiv.set_items(&fiv, items, 0);
	    fiv.set_text(&fiv,&software_info);
		//vi.set_btn(&vi, NULL, NULL, &rbtn);
 
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
		boot_modem(0);
#endif  /* MTK_DT_SUPPORT */ 

#ifdef ENTER_TP_TEST
            param.name = get_item_name(get_item_list(), ITEM_LCD);
            ftm_entry(ITEM_LCD, &param);
            param.name = get_item_name(get_item_list(), ITEM_TOUCH);
            ftm_entry(ITEM_TOUCH, &param);
#endif
		
		while (!exit) 
		{
			int chosen_item = fiv.run(&fiv, NULL);
			switch (chosen_item) 
			{
        case ITEM_MMI_TEST:
        case ITEM_PCBA_TEST:
            if(chosen_item == ITEM_PCBA_TEST){
                g_pcba_test = true;
            }else{
                g_pcba_test = false;
            }
            test_mode_menu(buf, BUFSZ);
            break;
            
			case ITEM_FULL_TEST:
#ifdef FEATURE_BATTERY_LOW_LEVEL            
				if(isBattLowLevel != true)
#endif                
				{
					full_test_mode(buf, BUFSZ);
				}
				break;
			case ITEM_AUTO_TEST:
				auto_test_mode(buf, BUFSZ);
				item_test_report(get_auto_item_list(), buf, BUFSZ);
				break;
			case ITEM_ITEM_TEST:
#ifdef FEATURE_BATTERY_LOW_LEVEL            
				if(isBattLowLevel != true)
#endif                
				{
					item_test_mode(buf, BUFSZ);
				}
				break;
			case ITEM_DEBUG_TEST:
				debug_test_mode(buf, BUFSZ);
				break;
			case ITEM_REPORT:
				item_test_report(get_item_list(), buf, BUFSZ);
				break;
			case ITEM_VERSION:
                display_version(0);				
				break;
    		case ITEM_HARDWARE_INFO:
#if defined(FEATURE_FTM_EMMC) && defined(FEATURE_FTM_ROM_CAPACITY)
				{
				char *pram = NULL;
				int total_size = sizeof(struct eMMC);
	
				pram = malloc(total_size);
				if(pram)
				{
					memset(pram, 0, total_size);
				}
				init_text(&info, pram, COLOR_YELLOW);
				create_hardwareinfo((char *)pram, total_size);				
				}
#else
                init_text(&info, buf, COLOR_YELLOW);//zhaoqf_sh
                create_hardwareinfo(buf, BUFSZ);
#endif				
                hw_info.set_title(&hw_info, &hw_info_tittle);
                hw_info.set_text(&hw_info, &info);
                LOGE("baal add test");
                hw_info.m_pstr = trans_verinfo(info.string, &nr_line);
                hw_info.m_nr_lines = g_nr_lines;
                LOGE("g_nr_lines is %d, avail_lines is %d\n", g_nr_lines, avail_lines);
                hw_info.m_start = 0;
                hw_info.m_end = (nr_line < avail_lines ? nr_line : avail_lines);
                LOGE("hw_info.m_end is %d\n", hw_info.m_end);
                hw_info.run(&hw_info);
                tear_down(hw_info.m_pstr, g_nr_lines);
                break;
#if 1//zhaoqf_sh change reboot to power off
        case ITEM_POWEROFF:
#else
        case ITEM_REBOOT:
#endif            
				exit = 1;
				fiv.exit(&fiv);
				break;
// sunxuewen added for htc-cid select tool begin
	case ITEM_HTC_CID_SELETC:
		ftm_htc_cid_select_mode(buf, BUFSZ);
		break;
// sunxuewen added for htc-cid select tool end
			default:
				param.name = get_item_name(items, chosen_item);
				ftm_entry(chosen_item, &param);
				break;
			}
		}//end while

		if (buf)
			free(buf);

		ui_printf("Entering factory reset mode...\n");
	ui_printf("Power off ...\n");//("Rebooting...\n");//zhaoqf_sh
	sync();
	reboot(RB_POWER_OFF);//(RB_AUTOBOOT);//zhaoqf_sh

		return EXIT_SUCCESS;
	}
	else
	{
		LOGE(TAG "Unsupported Factory mode\n");
	}
	
	return EXIT_SUCCESS;
}
