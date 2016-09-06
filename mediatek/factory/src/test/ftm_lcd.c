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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <common.h>
#include <miniui.h>
#include <ftm.h>

#include <cust_lcd.h>

#ifdef FEATURE_FTM_LCD

#define BRIGHTNESS_FILE "/sys/class/leds/lcd-backlight/brightness"

static int seq_index = 0;
static int brightness_seq[] = {
	50, 100, 150, 200, 255, -1
};

#define TAG	"[LCD] "
#define mod_to_lcd(p)	(lcd_module *)((char *)(p) + sizeof(struct ftm_module))

#define ARY_SIZE(x)     (sizeof((x)) / sizeof((x[0])))

enum {
	ITEM_PASS,
	ITEM_FAIL,
	ITEM_SHOW_TEST_IMAGES,
	ITEM_CHANGE_CONTRAST
};

static item_t lcd_item[] = {
	item(ITEM_SHOW_TEST_IMAGES, uistr_info_show_test_images),
	item(ITEM_CHANGE_CONTRAST, uistr_info_change_contrast),
	item(ITEM_PASS,             uistr_pass),
	item(ITEM_FAIL,             uistr_fail),
	item(-1, NULL),
};

DEFINE_TEST_IMAGE_FILENAMES(TEST_IMAGES);

typedef struct {
	struct ftm_module *module;

    /* image view */
    struct imageview img_view;
    char img_title[128];
    char img_filename[128];
    text_t img_title_ctext;
    unsigned int img_index;

    /* item view */
	struct itemview *itm_view;
    text_t itm_title_ctext;
} lcd_module;


static int
write_int(char const* path, int value)
{
	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = sprintf(buffer, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		return amt == -1 ? -errno : 0;
	}

	LOGE("write_int failed to open %s\n", path);
	return -errno;
}

static void change_contrast(void)
{
	write_int(BRIGHTNESS_FILE, brightness_seq[seq_index]);
	if (brightness_seq[++seq_index] == -1)
		seq_index = 0;
}

static void update_imageview(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;
    unsigned int i = lcd->img_index;

    LOGD(TAG "update_imageview(%d)\n", i);

    sprintf(lcd->img_title, "LCD Test Image (%d/%d)", 
            i + 1, ARY_SIZE(TEST_IMAGES));

    sprintf(lcd->img_filename,
            "/res/images/%s.png", TEST_IMAGES[i]);

    init_text(&lcd->img_title_ctext, lcd->img_title, COLOR_YELLOW);

    //iv->set_title(iv, &lcd->img_title_ctext);
    iv->set_title(iv, NULL);
    iv->set_image(iv, lcd->img_filename, 0, 0);
}


static int lcd_imageview_key_handler(int key, void *priv) 
{
    lcd_module *lcd = (lcd_module *)priv;
	struct imageview *iv = &lcd->img_view;
    int handled = 0;
    bool exit = false;

    switch (key)
    {
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
        if (lcd->img_index > 0) {
            -- lcd->img_index;
        }
        break;

    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
        if (lcd->img_index < (ARY_SIZE(TEST_IMAGES) - 1)) {
            ++ lcd->img_index;
        }
        break;

    case UI_KEY_BACK:
	case UI_KEY_CONFIRM:
        exit = true;
        break;

    default:
        handled = -1;
        break;
    }

    if (exit) {
        iv->exit(iv);
    } else if (-1 != handled) {
        update_imageview(lcd);
    }
    
    return handled;
}


static void display_test_images(lcd_module *lcd)
{
	struct imageview *iv = &lcd->img_view;

    ui_init_imageview(iv, lcd_imageview_key_handler, lcd);

    lcd->img_index = 0;
    update_imageview(lcd);
    iv->run(iv);
}


static void init_main_menu(struct ftm_param *param, lcd_module *lcd)
{
    struct itemview *iv = lcd->itm_view;

    init_text(&lcd->itm_title_ctext, param->name, COLOR_YELLOW);

    iv->set_title(iv, &lcd->itm_title_ctext);
    iv->set_items(iv, lcd_item, ITEM_SHOW_TEST_IMAGES);
}

/*zhaoqf_sh add for full test new mode   start*/
#define VIBRATOR_ENABLE 	"/sys/class/timed_output/vibrator/enable"
//#define BUTTON_LED_FILE	    "/sys/class/leds/button-backlight/brightness"
static void full_test(struct ftm_param *param,lcd_module *lcd)
{
    int key=0,k=3,i=0;
    int cwidth =gr_fb_width()/3;
    int cheight = gr_fb_height();
    bool exit=false;
    text_t itm_title;
    struct ftm_module *mod;

    /*view R G B base color*/
    //write_int(BUTTON_LED_FILE,255); //turn on the key-pad backlight
    while(k){
        switch(k){
        case 3:
            ui_color(255,0,0,255);
            break;
        case 2:
            ui_color(0,255,0,255);
            break;
        case 1:
            ui_color(0,0,255,255);
            break;
        }
        ui_fill(0,0,gr_fb_width(), cheight);
        ui_flip();
        key = ui_wait_key();
        if(UI_KEY_CONFIRM==key){
            k--;
        }else if(key == get_back_fail_key()){
            lcd->module->test_result = FTM_TEST_FAIL;
            return;
        }
    }
    ui_clear_key_queue();
    ui_color(255,255,255,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
#ifndef FEATURE_FTM_VIBRATOR
    write_int(VIBRATOR_ENABLE, 60000); // vibrator work for 10 seconds
#endif    
    k = 0;
    while(k<255)
    {
    	if((255-k) < 30)
    	{
			k = 255 - 30;
		}
        write_int(BRIGHTNESS_FILE,255-k);
		LOGD(TAG "GET_cur_lcd_level_%d\n", 255-k);
		
        /*if(0==i%2){
            write_int(BUTTON_LED_FILE,0);
        }else{
            write_int(BUTTON_LED_FILE,255);
        }*/
        i++;
        k += 50;
        usleep(300000);
    }
    ui_clear_key_queue();
    //write_int(VIBRATOR_ENABLE, 0); 
    //write_int(BUTTON_LED_FILE,255);
    while(1){
        key = ui_wait_key();
        if(UI_KEY_CONFIRM==key){
            lcd->module->test_result = FTM_TEST_PASS;
            break;
        }else if(key == get_back_fail_key()){
            lcd->module->test_result = FTM_TEST_FAIL;
            break;
        }
    }
#ifndef FEATURE_FTM_VIBRATOR
    write_int(VIBRATOR_ENABLE, 0); 
#endif
    ui_color(0,0,0,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
    write_int(BRIGHTNESS_FILE, 255);
}
/*zhaoqf_sh add for full test new mode   end*/

#ifdef ENTER_TP_TEST    
static void lcd_test(struct ftm_param *param,lcd_module *lcd)
{
    int key=0,k=3,i=0;
    int cwidth =gr_fb_width()/3;
    int cheight = gr_fb_height();
    bool exit=false;
    text_t itm_title;
    struct ftm_module *mod;

    ui_color(255,0,0,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
    usleep(500000);

    ui_color(0,255,0,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
    usleep(500000);

    ui_color(0,0,255,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
    usleep(500000);

    ui_color(255,255,255,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
        
    while(k<255)
    {
    	if((255-k) < 30)
    	{
			k = 255 - 30;
		}
        write_int(BRIGHTNESS_FILE,255-k);
		
        i++;
        k += 50;
        usleep(300000);
    }
    ui_clear_key_queue();
    ui_color(0,0,0,255);
    ui_fill(0,0,gr_fb_width(), cheight);
    ui_flip();
    write_int(BRIGHTNESS_FILE, 255);
}
#endif

static int lcd_entry(struct ftm_param *param, void *priv)
{
	lcd_module *lcd = (lcd_module *)priv;

    bool exit = false;
    seq_index = 0;

#ifdef ENTER_TP_TEST    
    lcd_test(param,lcd);
    return 0;
#endif
/*zhaoqf_sh add for full test new mode   start*/
    {
    full_test(param,lcd);
    return 0;
    }
/*zhaoqf_sh add for full test new mode   end*/

    init_main_menu(param, lcd);
    while(!exit)
    {
        switch(lcd->itm_view->run(lcd->itm_view, NULL))
        {
        case ITEM_SHOW_TEST_IMAGES:
            display_test_images(lcd);
            init_main_menu(param, lcd);
            exit = false;
            break;
        case ITEM_CHANGE_CONTRAST:
	    change_contrast();
            exit = false;
            break;
        case ITEM_PASS:
            lcd->module->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            lcd->module->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case -1:
            exit = true;
            break;
        default:
            break;
        }
    }
    write_int(BRIGHTNESS_FILE, 255);

	return 0;
}


int lcd_init(void)
{
	int r;
	struct ftm_module *mod;
	lcd_module *lcd;

	write_int(BRIGHTNESS_FILE, 255);

	mod = ftm_alloc(ITEM_LCD, sizeof(lcd_module));
	if (!mod)
		return -ENOMEM;

	lcd = mod_to_lcd(mod);

	lcd->module = mod;
	lcd->itm_view = ui_new_itemview();
	if (!lcd->itm_view)
		return -ENOMEM;

	r = ftm_register(mod, lcd_entry, (void*)lcd);
	if (r) {
		LOGD(TAG "register LCD failed (%d)\n", r);
		return r;
	}

	return 0;
}

#endif