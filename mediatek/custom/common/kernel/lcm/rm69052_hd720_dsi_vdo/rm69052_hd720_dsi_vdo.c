#ifndef BUILD_LK
    #include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

#define LCM_ID_H497TLB (0x90)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	
        {0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x00}},//page 0
	{0xB0,	3,	{0x00, 0x10, 0x10}},
	{0xBA,	1,	{0x60}},
	{0xBB,	7,	{0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77}},
        {0xD5,	1,	{0x00}},
        {0xD6,	1,	{0x00}},
        {0xD7,	1,	{0x00}},

	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x02}}, //page 2
	{0xCA,	1,	{0x04}},
        {0xE1,	1,	{0x00}},//close dimming brightness
	{0xE2,	1,	{0x0A}},
	{0xE3,	1,	{0x40}},
	{0xE7, 4,{0x00,0x00,0x00,0x00}},
	{0xED,	8,	{0x48, 0x00, 0xE0, 0x13, 0x08, 0x00, 0x91, 0x08}},

	{0xFD,	6,	{0x00, 0x08, 0x1C, 0x00, 0x00, 0x01}},
	//data type =0x39
	{0xC3, 10,{0x11,0x24,0x04,0x0A,0x02,0x04,0x00,0x1C,0x10,0xF0}},

	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x03}}, //page 3
	{0xF1,	6,	{0x00, 0x00, 0x00, 0x00, 0x00, 0x01}},
	{0xF6,	1,	{0x08}},
        {0xE0,	1,	{0x00}},
	
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x05}}, //page 5
	
	{0xC4,	2,	{0x00, 0x14}},
	
	{0xC9,	1,	{0x04}},
	
	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x01}}, //page 1

	{0xB0,	3,	{0x06, 0x06, 0x06}},
	{0xB1,	3,	{0x14, 0x14, 0x14}},
	{0xB2,	3,	{0x00, 0x00, 0x00}},
	{0xB4,	3,	{0x66, 0x66, 0x66}},
	{0xB5,	3,	{0x44, 0x44, 0x44}},
	{0xB6,	3,	{0x54, 0x54, 0x54}},
	{0xB7,	3,	{0x24, 0x24, 0x24}},
	{0xB9,	3,	{0x04, 0x04, 0x04}},
	{0xBA,	3,	{0x14, 0x14, 0x14}},
	{0xBE,	3,	{0x23, 0x78, 0x78}},

        {0xCF,	7,	{0x90, 0xFF, 0xE0, 0xBB, 0xA9, 0x88, 0x6C}},

//	{0xF0,	5,	{0x55, 0xAA, 0x52, 0x08, 0x02}},
	



	{0x35, 1, {0x00}},//data type =0x15  ,1 parameter
	{0x11, 0, {0x00}},//data type =0x05 no parameter
	{REGFLAG_DELAY, 100, {}},
// Display ON
	{0x29, 0, {0x00}},
        {REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
// Note
// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
};



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
    // Sleep Mode On
	 {0x10, 1, {0x00}},
	 {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//UDELAY(5);//soso add or it will fail to send register
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

    params->dsi.mode   =  SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;  
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM = LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
    params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    	params->dsi.packet_size=256;

    //params->dsi.word_count=480*3;	//DSI CMD mode need set these two bellow params, different to 6577

    params->dsi.vertical_sync_active				= 2;    //4//4  16   20
    params->dsi.vertical_backporch					= 8;
    params->dsi.vertical_frontporch					= 8;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active				= 4;///10   50   60 
    params->dsi.horizontal_backporch				= 16;
    params->dsi.horizontal_frontporch				= 16;
    params->dsi.horizontal_blanking_pixel				= 60;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.PLL_CLOCK=235;
	params->dsi.ssc_range = 0;	
	params->dsi.ssc_disable = 1;
	

}

static unsigned int lcm_compare_id(void);

static void lcm_poweron(bool onoff)
{
	if(onoff == true)
	{
	    //set ouput 4.6/-4.4
		mt_set_gpio_out((GPIO90 | 0x80000000),1);
		MDELAY(10);
		mt_set_gpio_out((GPIO90 | 0x80000000),0);
		UDELAY(10);
		mt_set_gpio_out((GPIO90 | 0x80000000),1);
		MDELAY(10);
	}
	else
	{
	    //set ouput 0
		mt_set_gpio_out((GPIO90 | 0x80000000),0);
                MDELAY(10);
	}
}


static void lcm_init(void)
{
	unsigned int data_array[16];

	//lcm_poweron(true);
	
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(50);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	lcm_poweron(true);
}


static void lcm_suspend(void)
{

	unsigned int data_array[16];

	lcm_poweron(false);
	
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(10); 
	
	data_array[0] = 0x00063902;//0XF0
	data_array[1] = 0x52AA55F0;
    data_array[2] = 0x00000108; 				
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10);

	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x141414B0; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);

	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x000014B6; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	
	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x444444B4; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x222222B5; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	
	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x333333B4; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);	
	
    data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x111111B5; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	
	data_array[0] = 0x00043902;//0XCF
	data_array[1] = 0x000000B5; 				
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(100);

}
	

static void lcm_resume(void)
{
#if 1
	lcm_init();
#else
	unsigned int data_array[16];

	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(300);
	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

     //   lcm_poweron(true);
#endif
}

static void lcm_setBrightness(int level)
{

	unsigned int data_array[16];

	data_array[0] = 0x00063902;//0XF0
	data_array[1] = 0x52AA55F0;
        data_array[2] = 0x00000108; 				
	dsi_set_cmdq(data_array, 3, 1);
        MDELAY(10);
	data_array[0] = 0x00023902;//0XCF
	data_array[1] = 0x000000cf|((level&0xff)<<8); 				
	dsi_set_cmdq(data_array, 2, 1);
        MDELAY(10);

}


static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];

	//lcm_poweron(true);
	
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //we only need ID

	return 1;

}


LCM_DRIVER rm69052_hd720_dsi_vdo_lcm_drv = 
{
    .name			= "rm69052_hd720_dsi_vdo_lcm_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	.set_backlight  = lcm_setBrightness,
};

