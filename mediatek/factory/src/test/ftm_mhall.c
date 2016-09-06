
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

#ifdef WT_BU52051NVX_MHALL_SUPPORT

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[MHALL] "
#define mod_to_mhall_data(p) (struct mhall_data*)((char*)(p) + sizeof(struct ftm_module))

/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t mhall_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

static item_t mhall_auto_items[] = {
    item(-1, NULL),
};

/*---------------------------------------------------------------------------*/
struct mhall_priv
{
    /*specific data field*/
	char	*dev;
	int		fd;
	bool	avail;
	int		mhall_state; 
};

struct mhall_data
{
    struct mhall_priv mhall;
    /*common for each factory mode*/
    char  info[1024];
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    //struct textview tv;
    struct itemview *iv;
};

static bool thread_exit = false;

/*---------------------------------------------------------------------------*/

#define MHALL_STATE_PATH	"/sys/devices/platform/mhall/driver/mhall_state"
#define BUF_LEN 1
static char readbuf[BUF_LEN] = {'\0'};
static char leave_buf[BUF_LEN] = {'0'};
static char cover_buf[BUF_LEN] = {'1'}; 


/******************************************************************************
 * Functions 
 *****************************************************************************/
static int mhall_init_priv(struct mhall_priv *priv)
{
    memset(priv, 0x00, sizeof(*priv));
    priv->fd = -1;
    priv->dev = "/dev/mhall";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int mhall_open(struct mhall_priv *mhall)
{
    return 0;
}
/*---------------------------------------------------------------------------*/
static int mhall_close(struct mhall_priv *mhall)
{
    return 0;
}
/*---------------------------------------------------------------------------*/
static int mhall_update_info(struct mhall_priv *mhall)
{
	int fd = -1;

	usleep(20000);
	
	fd = open(MHALL_STATE_PATH, O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "Can't open %s\n", MHALL_STATE_PATH);
		mhall->avail = false;
		goto EXIT;
	}
	if (read(fd, readbuf, BUF_LEN) == -1) {
		LOGD(TAG "Can't read %s\n", MHALL_STATE_PATH);
		mhall->avail = false;
		goto EXIT;
	}
	if (!strncmp(leave_buf, readbuf, BUF_LEN)) { /*leave*/
		mhall->avail = true;
		mhall->mhall_state = 0;
    }
	else if(!strncmp(cover_buf, readbuf, BUF_LEN)) { /*cover*/
		mhall->avail = true;
		mhall->mhall_state = 1;
	}
	else{
    	mhall->avail = false;
    }
	
EXIT:
	close(fd);	
	
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *mhall_update_iv_thread(void *priv)
{
    struct mhall_data *dat = (struct lps_data *)priv; 
    struct mhall_priv *mhall = &dat->mhall;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = mhall_open(mhall))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, uistr_info_sensor_init_fail);
        iv->redraw(iv);
		mhall_close(mhall);
        LOGD("mhall_open() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1){
		
        if (dat->exit_thd)
            break;
            
        if (err = mhall_update_info(mhall))
            continue;     

    /* preare text view info */
	len = 0;
    if(!mhall->avail){
       len += snprintf(dat->info+len, sizeof(dat->info)-len,"%s\n\n",uistr_info_audio_headset_note);
    }

    len += snprintf(dat->info+len, sizeof(dat->info)-len,"%s : %s\n\n",uistr_info_mhall_state ,mhall->mhall_state ? "Cover" : "Leave");

	iv->set_text(iv, &dat->text);
    iv->redraw(iv);

	int status = get_is_ata();
        if(status == 1)
        { 
            thread_exit = true;  
            break;
        }

    }
    mhall_close(mhall);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}

/*---------------------------------------------------------------------------*/
int mhall_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct mhall_data *dat = (struct mhall_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;

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
	if(is_full_mode) {
	iv->set_items(iv, mhall_auto_items, 0);
	}
	else {
	iv->set_items(iv, mhall_items, 0);
	}
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, mhall_update_iv_thread, priv);
    do {
        if(get_is_ata() != 1){
        chosen = iv->run(iv, &thread_exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }           
            thread_exit = true;          
            break;
        }
        }
        iv->redraw(iv);
        if (thread_exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int mhall_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct mhall_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_MHALL_SENSOR, sizeof(struct mhall_data));
    dat  = mod_to_mhall_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    mhall_init_priv(&dat->mhall);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, mhall_entry, (void*)dat);

    return ret;
}

#endif 

