

#include "cid.h"
#include "libnvram.h"
#include "Custom_NvRam_LID.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "common.h"
#include "miniui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/properties.h>
#include <dirent.h>
#include <alloca.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/mount.h>


#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "CID_USE"
#include <cutils/log.h>


#define NULL 0
#define BUF_SIZE 1024

static char const* PROPERTY_CID = "ro.cid";
static char const* RESOURCE_PATH = "/system/htc_resource/";
static char const* BACKUP_PATH = "/system/htc_resource/backup";
static char const* SUCCEED_TAG = "/data/.cidsuccess";
static char const* CID_BACKUP_VALUE = "/data/.cidbackup";


char CID_AVAILABLE[16][9];
int CID_AVAILABEL_COUNT = 0;

static item_t cid_ret_items[] = 
{
	item_cid(ITEM_HTC_CID_English_001,         uistr_CID_English_001   ),
	item_cid(ITEM_HTC_CID_German_102,         uistr_CID_German_102   ),
};

void cid_default_value_get(char *cid)
{
	if(NULL == cid)
	{
		ALOGD("error: get_default_cid: cid is null\n");
		return;
	}
	
	char *fileName = "/system/htc_resource/cid_default";

	FILE  *fp = fopen(fileName, "r");

        char buf[BUF_SIZE] = {0};

        if(NULL == fp)
        {
                ALOGD("file[%s] open error\n", fileName);
                return;
        }

	fgets(buf, BUF_SIZE, fp);
	memcpy(cid, buf, 8);
	ALOGD("default  cid is[%s]\n", cid);
	
	fclose(fp);
}


void cid_backup_value_get(char *cid)
{
	if(NULL == cid)
	{
		ALOGD("error: cid_backup_value_get: cid is null\n");
		return;
	}
	

	FILE  *fp = fopen(CID_BACKUP_VALUE, "r");

        char buf[BUF_SIZE] = {0};

        if(NULL == fp)
        {
                ALOGD("file[%s] open error\n", CID_BACKUP_VALUE);
                return;
        }

	fgets(buf, BUF_SIZE, fp);
	memcpy(cid, buf, 8);
	ALOGD("backup  cid is[%s]\n", cid);
	
	fclose(fp);
}


int cid_write_nv(char *cid)
{	
	if(NULL == cid)
	{
		ALOGE("error: cid_write cid is null\n");
		return -1;
	}

	//printf("cid_write: cid is[%s]\n", cid);
	ALOGD("cid_write: cid is[%s]\n", cid);
	
	
	int file_lid = AP_CFG_REEB_PRODUCT_INFO_LID;
	int rec_size;
	int rec_num;
	int res;
	
	PRODUCT_INFO htc_cid;
	memset(&htc_cid, 0, sizeof(PRODUCT_INFO));
	
	F_ID  fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	if(fd.iFileDesc < 0) 
	{
		ALOGE("error: cid_write NVM_GetFileDesc  error");
		//printf("ERROR: cid_write NVM_GetFileDesc  error\n");
		return fd.iFileDesc;
	}

	res = read(fd.iFileDesc, &htc_cid , rec_size*rec_num);
	if(res < 0)
	{
		ALOGE("ERROR: cid_write cid read error");
		//printf("ERROR: cid_write cid read error\n");
		NVM_CloseFileDesc(fd); 
		return -1;
	}
	
	NVM_CloseFileDesc(fd); 

	
	fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	if(fd.iFileDesc < 0) 
	{
		ALOGE("ERROR: cid_write  NVM_GetFileDesc  error");
		//printf("ERROR: cid_write NVM_GetFileDesc  error\n");
		return fd.iFileDesc;
	}

	memcpy(&(htc_cid.reserved[100]), cid, strlen(cid));

	res = write(fd.iFileDesc, &htc_cid, rec_size*rec_num);
	if(res < 0)
	{
		ALOGE("ERROR: cid_write cid read error");
		//printf("ERROR: cid_write cid read error\n");
		NVM_CloseFileDesc(fd); 
		return -1;
	}
	
	NVM_CloseFileDesc(fd); 
	
	return 0;
}

int cid_read_nv(char *cid)
{
	if(NULL == cid)
	{
		ALOGE("error: cid_read cid is null\n");
		return -1;
	}
	
	int file_lid = AP_CFG_REEB_PRODUCT_INFO_LID;
	int rec_size;
	int rec_num;
	int res;
	
	PRODUCT_INFO htc_cid;
	memset(&htc_cid, 0, sizeof(PRODUCT_INFO));
	
	F_ID  fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	//ALOGE("get fd");

	if(fd.iFileDesc < 0) 
	{
		ALOGE("cid_read NVM_GetFileDesc  error");
		printf("cid_read NVM_GetFileDesc  error\n");
		return fd.iFileDesc;
	}

	res = read(fd.iFileDesc, &htc_cid , rec_size*rec_num);

	if(res < 0)
	{
		ALOGE("cid_read read error");
		printf("cid_read read error\n");
		NVM_CloseFileDesc(fd); 
		return -1;
	}
	else
	{
		//ALOGD("cid_read cid is [%s]", htc_cid.reserved);
		memset(cid, 0, 9);
		memcpy(cid, &(htc_cid.reserved[100]), 8);
		ALOGD("cid_read get cid is[%s]\n", cid);
	}


	NVM_CloseFileDesc(fd); 

	return 0;
}

int cid_modify(char * cid)
{
	if(NULL == cid)
	{
		ALOGE("cid_modify  error: cid is null");
		printf("cid_modify  error: cid is null\n");
		
		return -1;
	}
	
	char cid_nv[9] = {0};
	cid_read_nv(cid_nv);

	if(0 == strcmp(cid, cid_nv))
	{	
		ALOGE("cid_modify  error: cid is same");
		return -2;
	}

	return cid_write_nv(cid);
}

void cid_available_values_get()
{
	memset(CID_AVAILABLE, 0, sizeof(CID_AVAILABLE));
	CID_AVAILABEL_COUNT  = 0;
		
	DIR *pdir = opendir(RESOURCE_PATH);
	if(NULL == pdir)
	{
	        ALOGE("cid_available_values_get:open dir [%s] error\n", RESOURCE_PATH);
	        return;
	}

	struct dirent *pdirent =  NULL;


	while(pdirent = readdir(pdir))
	{
		
		if(0 == strncmp("HTC__", pdirent->d_name, strlen("HTC__")))
		{
			CID_AVAILABEL_COUNT++;
		}
	}

	 printf("cid_available_values_get:available cid count[%d]\n", CID_AVAILABEL_COUNT);
	 //CID_AVAILABLE = (char **)malloc(CID_AVAILABEL_COUNT * sizeof(char*));


	closedir(pdir);
	pdirent = NULL;
	
	pdir = opendir(RESOURCE_PATH);

	int i = 0;
	while(pdirent = readdir(pdir))
	{	
		if(0 == strncmp("HTC__", pdirent->d_name, strlen("HTC__")))
		{
			strcpy(&(CID_AVAILABLE[i]), pdirent->d_name);
			printf("cid_available_values_get: cid[%d][%s]\n", i, CID_AVAILABLE[i]);
			i++;
		}
	}

	closedir(pdir);
	return;
}

int is_cid_in_available(char *cid)
{
	int ret = 0;

	if(NULL == cid)
	{
		ALOGE("is_cid_in_available:cid is null\n");
		return 0;
	}
	
	cid_available_values_get();
	ALOGD("is_cid_in_available: count[%d]", CID_AVAILABEL_COUNT);
	printf("is_cid_in_available: count[%d]\n", CID_AVAILABEL_COUNT);
	
	int i = 0;
	for(i = 0; i < CID_AVAILABEL_COUNT; i++)
	{
		printf("is_cid_in_available: cid_available[%d][%s]\n", i, CID_AVAILABLE[i]);
		ALOGD("is_cid_in_available: cid_available[%d][%s]\n", i, CID_AVAILABLE[i]);
		if(0 == strcmp(cid, CID_AVAILABLE[i]))
		{
			ret = 1;
			break;
		}
	}
	

	printf("is_cid_in_available: cid[%s][%s]\n", cid, ret == 1 ? "true" : "false");
	ALOGD("is_cid_in_available: cid[%s][%s]\n", cid, ret == 1 ? "true" : "false");
	return ret;
}

int get_current_cid(char *current_cid)
{
	char cid_nv[9] = {0};
	char cid_dft[9] = {0};
	char cid_bk[9] = {0};
	
	if(NULL == current_cid){
		return -1;
	}

	cid_read_nv(cid_nv);
	cid_default_value_get(cid_dft);
	cid_backup_value_get(cid_bk);

	// has been powered on to android
	if(1 == is_cid_in_available(cid_bk))
	{
		memcpy(current_cid, cid_bk, 8);
		return 0;
	}

	// only flashed, and then go to factory
	if(1 == is_cid_in_available(cid_nv))
	{
		memcpy(current_cid, cid_nv, 8);
		return 0;
	}
	
	if(1 == is_cid_in_available(cid_dft))
	{
		memcpy(current_cid, cid_dft, 8);
		return 0;
	}

	memcpy(current_cid, cid_dft, 8);
	return -1;
}


static int cid_key_handler(int key, void *priv) 
{
    int handled = 0;
	struct textview *tv = (struct textview *)priv;

	ALOGD("cid_key_handler");

    switch (key) {
    case UI_KEY_BACK:
        tv->exit(tv);
        break;
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
        break;
    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}


int cid_draw_item(int isSuccess, char *cid)
{
	struct itemview itv;
	text_t  title;
	text_t  ret;
	item_t *items;
	bool exit = false;
	

	ALOGD("cid_draw: [%d], %s", isSuccess, cid);

	items = cid_ret_items;

	ui_init_itemview(&itv);
	init_text(&title, cid, COLOR_WHITE);
	init_text(&ret, "ok", COLOR_WHITE);
	itv.set_title(&itv, &title);
	itv.set_items(&itv, items, 0);
	itv.set_text(&itv, &ret);
	

	itv.run(&itv, &exit);
	if(exit == true)
	{
		itv.exit(&itv);
	}
	ALOGD("cid_draw end");

	return 0;
}


static int cid_text_key_handler(int key, void *priv) 
{
    int handled = 0;
    struct textview *tv = (struct textview *)priv;

    switch (key) {
    case UI_KEY_BACK:
        tv->exit(tv);
        break;
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
        break;
    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}

int cid_draw_text(int isSuccess, char *cid)
{
	struct textview vi;
	text_t title;
	text_t text;
	text_t rbtn;

	ctext_t ctexts[] = { 
		{ "AAA", COLOR_YELLOW, 20, 40 },
		{ "BBB", COLOR_GREEN, 20, 80 }, 
		{ "CCC", COLOR_RED, 20, 120 },
		{ "DDD", COLOR_BLUE, 20, 160 }, 
	};
	
	init_text(&title, cid, COLOR_WHITE);
	if(isSuccess == 0)
	{
		init_text(&rbtn, "OK", COLOR_GREEN);
	}
	else
	{
		init_text(&rbtn, "NG", COLOR_RED);
	}
	ui_init_textview(&vi, cid_text_key_handler, &vi);
	vi.set_title(&vi, &title);
	vi.set_btn(&vi, NULL, &rbtn, NULL);
	//vi.set_ctext(&vi, ctexts, 4);
	vi.run(&vi);

	return 0;
}


int cid_draw_device_info()
{
	struct textview vi;
	text_t title;
	text_t text;
	text_t rbtn;
	ctext_t ctexts[3];
	

	
	char cid[32] = {0};
	char *current_cid = "Current CID: ";
	memcpy(cid, current_cid, strlen(current_cid));
	get_current_cid(&(cid[strlen(current_cid)]));
	
	
	init_text(&title, uistr_CID_Device_Info, COLOR_WHITE);
	int x = 20;
	int y = 40;
	init_ctext(&(ctexts[0]), cid, COLOR_WHITE, x, y);
	//y = 80;
	//init_ctext(&(ctexts[1]), cid, COLOR_WHITE, x, y);
	//y = 120;
	//init_ctext(&(ctexts[2]), cid, COLOR_WHITE, x, y);
	ui_init_textview(&vi, cid_text_key_handler, &vi);
	vi.set_title(&vi, &title);
	//vi.set_btn(&vi, NULL, &rbtn, NULL);
	vi.set_ctext(&vi, ctexts, 1);
	vi.run(&vi);

	return 0;
}

