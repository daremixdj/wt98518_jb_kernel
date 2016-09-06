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
#include <sys/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#define TAG			"[wt_version]"
#define BUFSIZ      1024

enum {
	ITEM_PASS,
	ITEM_FAIL,
};

item_t wt_version_items[] = {
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

item_t wt_version_auto_items[] = {
    item(-1, NULL),
};

struct wtVersion {
	pthread_t update_thd;
	struct ftm_module *mod;
	struct itemview *iv;
	bool exit_thd;
	text_t title;
	char  info[BUFSIZ];
	text_t    text;
};

#define mod_to_wtVersion(p)     (struct wtVersion*)((char*)(p) + sizeof(struct ftm_module))

extern bool get_wtVersion_is_ready(void);
extern int get_SoftWareinfo(char *info, int size);

static void version_Info_update(struct wtVersion *wv, char *info)
{
	get_SoftWareinfo(info, BUFSIZ);
	return ;
}

static void *wt_version_update_thread(void *priv)
{
	struct wtVersion*versionInfo = (struct wtVersion*)priv;

	LOGD(TAG "%s: Start\n", __FUNCTION__);

	while (1)
	{
		usleep(200000);
		if(versionInfo->exit_thd)
		{
			break;
		}		
	}
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
	return NULL;	
}

int wt_version_entry(struct ftm_param *param, void *priv)
{
	bool exit = false;
	struct wtVersion *dat = (struct wtVersion *)priv;
	struct itemview *iv;
	int chosen = 0;

    LOGD(TAG "%s\n", __FUNCTION__);

	init_text(&dat->title, param->name, COLOR_YELLOW);
	init_text(&dat->text, &dat->info[0], COLOR_YELLOW);

	dat->exit_thd = false;
	
    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "wt_version No memory");
            return -1;
        }
        dat->iv = iv;
    }
    iv = dat->iv;
    iv->set_title(iv, &dat->title);
	if(is_full_mode)
	{
	iv->set_items(iv, wt_version_auto_items, 0);
	}
	else
	{
	iv->set_items(iv, wt_version_items, 0);
	}
    iv->set_text(iv, &dat->text);

	version_Info_update(dat, dat->info);

	pthread_create(&dat->update_thd, NULL, wt_version_update_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
			if(chosen == ITEM_PASS)
			{
				if(get_wtVersion_is_ready())
				{
					dat->mod->test_result = FTM_TEST_PASS;
				}
				else
				{
					if(is_full_mode)
					{
						continue;
					}
					else
					{
						dat->mod->test_result = FTM_TEST_FAIL;
						break;
					}
				}
			}
			else if (chosen == ITEM_FAIL)
			{
				dat->mod->test_result = FTM_TEST_FAIL;
			}
			exit = true;
            break;
		default:
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

int wt_version_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct wtVersion *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_VERSION, sizeof(struct wtVersion));
    dat = mod_to_wtVersion(mod);	
    memset(dat, 0x00, sizeof(*dat));
    dat->mod = mod; 
    if (!mod)
    {
    	return -ENOMEM;
    }
    ret = ftm_register(mod, wt_version_entry, (void*)dat);

    return ret;
}

