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

/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "VMC8178AF.h"
#include "../camera/kd_camera_hw.h"
//#include "kd_cust_lens.h"

//#include <mach/mt6573_pll.h>
//#include <mach/mt6573_gpt.h>
//#include <mach/mt6573_gpio.h>

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("VMC8178AF", 0x49)};

#define VMC8178AF_DRVNAME "VMC8178AF"
#define VMC8178AF_VCM_WRITE_ID           0x49

#define VMC8178AF_DEBUG
#ifdef VMC8178AF_DEBUG
#define VMC8178AFDB printk
#else
#define VMC8178AFDB(x,...)
#endif

char VASSTEK[] = {
//00  ,01  ,02  ,03  ,04  ,05  ,06  ,07  ,08  ,09
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//10  ,11  ,12  ,13  ,14  ,15  ,16  ,17  ,18  ,19
0x00,0x10,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//20  ,21  ,22  ,23  ,24  ,25  ,26  ,27  ,28  ,29
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xf5,
//30  ,31  ,32  ,33  ,34  ,35  ,36  ,37  ,38  ,39
0x00,0x28,0x1f,0x07,0xff,0x7f,0xff,0x7f,0x00,0x85,
//40  ,41  ,42  ,43  ,44  ,45  ,46  ,47  ,48  ,49
0x64,0x00,0xff,0x7f,0x00,0x00,0x13,0x0f,0x70,0xfe,
//50  ,51  ,52  ,53  ,54  ,55  ,56  ,57  ,58  ,59
0xFF,0x7F,0xff,0x58,0x02,0x00,0x80,0x80,0xc1,0xc0,
//60  ,61  ,62  ,63  ,64  ,65  ,66  ,67  ,68  ,69
0xf9,0x40,0x1f,0xc0,0x5d,0xff,0x7f,0x00,0x80,0x80,
//70  ,71  ,72  ,73  ,74  ,75  ,76  ,77  ,78  ,79
0xc1,0xc0,0xf9,0x40,0x1f,0xc0,0x5d,0xff,0x7f,0x00,
//80  ,81  ,82  ,83  ,84  ,85  ,86  ,87  ,88  ,89
0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x23,
//90  ,91  ,92  ,93  ,94  ,95  ,96  ,97  ,98  ,99
0x00,0xb0,0x04,0x2d,0x00,0x90,0x01,0x41,0x00,0xff,
//100,101, 102, 103, 104, 105, 106, 107, 108, 109
0x64,0x50,0x00,0x14,0x00,0xA0,0x0F,0xff,0x1f,0x00,
//110,111, 112, 113, 114, 115, 116, 117, 118, 119
0x00,0x00,0x00,0x00,0x00,0x40,0x00,0xdf,0x1f,0x10,
//120,121, 122, 123, 124, 125, 126, 127, 128, 129
0x10,0x26,0x02,0x5e,0x01,0xff,0xff,0xff,0x7f,0x4e,
//130,131, 132, 133, 134, 135, 136, 137, 138, 139
0x3e,0x8b,0x0d,0x4e,0x3e,0x8b,0x0d,0xe8,0x1b,0x0b,
//140,141, 142, 143, 144, 145, 146, 147, 148, 149
0x12,0xe8,0x1b,0x0b,0x12,0x00,0x4b,0x90,0x01,0x00,
//150,151, 152, 153, 154, 155, 156, 157, 158, 159
0x00,0xbc,0x34,0x00,0x00,0x40,0x00,0x00,0x00,0x00,
//160,161, 162, 163, 164, 165, 166, 167, 168,
0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x04
};
static spinlock_t g_VMC8178AF_SpinLock;
/* Kirby: remove old-style driver
static unsigned short g_pu2Normal_VMC8178AF_i2c[] = {VMC8178AF_VCM_WRITE_ID , I2C_CLIENT_END};
static unsigned short g_u2Ignore_VMC8178AF = I2C_CLIENT_END;

static struct i2c_client_address_data g_stVMC8178AF_Addr_data = {
    .normal_i2c = g_pu2Normal_VMC8178AF_i2c,
    .probe = &g_u2Ignore_VMC8178AF,
    .ignore = &g_u2Ignore_VMC8178AF
};*/

static struct i2c_client * g_pstVMC8178AF_I2Cclient = NULL;

static dev_t g_VMC8178AF_devno;
static struct cdev * g_pVMC8178AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4VMC8178AF_Opened = 0;
static int  g_s4VMC8178AF_ReadId= 0;   //pengjinqiang tst

static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static long g_i4Position = 0;

static unsigned long g_u4VMC8178AF_INF = 0;
static unsigned long g_u4VMC8178AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static unsigned short PositionLow=0;
static unsigned short PositionHigh=0;

//static struct work_struct g_stWork;     // --- Work queue ---
//static XGPT_CONFIG	g_GPTconfig;		// --- Interrupt Config ---


//extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
//extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
//extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);

static int s4VMC8178AF_Read_ID(unsigned short * a_pu2Result)
{
#if 1//for VMC8178 Read
int i4RetValue1 = 0,i4RetValue2 = 0,i4RetValue3 = 0,i4RetValue4 = 0;


char pBuff1[1]={0x00},pBuff2[1]={0x01},pBuff3[1],pBuff4[1];

	g_pstVMC8178AF_I2Cclient->addr=0x92>>1;
	i4RetValue1 = i2c_master_send(g_pstVMC8178AF_I2Cclient, pBuff1, 1);

	i4RetValue2 = i2c_master_recv(g_pstVMC8178AF_I2Cclient, pBuff3 , 1);

	i4RetValue3 = i2c_master_send(g_pstVMC8178AF_I2Cclient, pBuff2, 1);

	i4RetValue4 = i2c_master_recv(g_pstVMC8178AF_I2Cclient, pBuff4 , 1);
	VMC8178AFDB("[VMC8178AF Read ID] I2C read i4RetValue1 = %d,i4RetValue2 = %d,i4RetValue3 = %d,i4RetValue4 = %d\n",i4RetValue1,i4RetValue2,i4RetValue3,i4RetValue4);

	if ((i4RetValue1 < 0)||(i4RetValue2 < 0)||(i4RetValue3 < 0)||(i4RetValue4 < 0)) 
	{
		VMC8178AFDB("[VMC8178AF] I2C read failed!! \n");
		return -1;
	}

	*a_pu2Result = ((((u16)pBuff4[0]) << 8)&0xFF00) |(pBuff3[0]);


	


return 0;
#endif
}




static int s4VMC8178AF_ReadReg(unsigned short * a_pu2Result)
{
#if 1//for VMC8178 Read
int i4RetValue1 = 0,i4RetValue2 = 0,i4RetValue3 = 0,i4RetValue4 = 0;


char pBuff1[1]={0xB3},pBuff2[1]={0xB4},pBuff3[1],pBuff4[1];


//char puSendCtrlCmd[2] = {(char)(0x1F) , 0x28};   // Servo Off

	g_pstVMC8178AF_I2Cclient->addr=0x92>>1;

	// i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd, 2);
	 
	i4RetValue1 = i2c_master_send(g_pstVMC8178AF_I2Cclient, pBuff1, 1);

	i4RetValue2 = i2c_master_recv(g_pstVMC8178AF_I2Cclient, pBuff3 , 1);

	i4RetValue3 = i2c_master_send(g_pstVMC8178AF_I2Cclient, pBuff2, 1);

	i4RetValue4 = i2c_master_recv(g_pstVMC8178AF_I2Cclient, pBuff4 , 1);
	VMC8178AFDB("[VMC8178AF] I2C read i4RetValue1 = %d,i4RetValue2 = %d,i4RetValue3 = %d,i4RetValue4 = %d\n",i4RetValue1,i4RetValue2,i4RetValue3,i4RetValue4);

	if ((i4RetValue1 < 0)||(i4RetValue2 < 0)||(i4RetValue3 < 0)||(i4RetValue4 < 0)) 
	{
		VMC8178AFDB("[VMC8178AF] I2C read failed!! \n");
		return -1;
	}


	*a_pu2Result = ((((u16)pBuff4[0]) << 8)&0xFF00)|(pBuff3[0]);


	


return 0;
#endif
}

static int s4VMC8178AF_WriteReg(u16 a_u2Data)
{
#if 1//for VMC8178 Write Position command
int i4RetValue1 = 0,i4RetValue2 = 0;
VMC8178AFDB("[VMC8178AF] AFPosition= %d\n",a_u2Data);
//char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+0xF)};
//** char puSendCmd1[2] = {(char)(0x04) , (char)((a_u2Data & 0x0300)>> 8)};
//** char puSendCmd2[2] = {(char)(0x05) , (char)(a_u2Data & 0x00FF)};
char puSendCmd1[2] = {(char)(0x03) , (char)(a_u2Data & 0x00FF)};
char puSendCmd2[2] = {(char)(0x04) , (char)((a_u2Data & 0xFF00)>> 8)};
//char puSendCtrlCmd1[2] = {(char)(0x1F) , 0xA8};   // Servo On , set mode
//char puSendCtrlCmd2[2] = {(char)(0x1F) , 0x28};   // Servo Off
g_pstVMC8178AF_I2Cclient->addr=0x92>>1;

    //     i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd1, 2);

i4RetValue1 = i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCmd1, 2);
i4RetValue2 = i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCmd2, 2);

 //i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd2, 2);
 
//mt_set_gpio_out(97,0);
if ((i4RetValue1 < 0)||(i4RetValue2 < 0))
{
VMC8178AFDB("[VMC8178AF] I2C send failed!! \n");
return -1;
	}

   

#endif

 return 0;
}


void GetBoundary(void)
{   
s4VMC8178AF_WriteReg(0);
 mdelay(200);
s4VMC8178AF_ReadReg(&PositionLow); 
s4VMC8178AF_WriteReg(4095);
 mdelay(200);
s4VMC8178AF_ReadReg(&PositionHigh); 

}



inline static int getVMC8178AFInfo(__user stVMC8178AF_MotorInfo * pstMotorInfo)
{
    stVMC8178AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4VMC8178AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4VMC8178AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;

  VMC8178AFDB("[VMC8178AF] getVMC8178AFInfo \n");
		
	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = TRUE;}
	else						{stMotorInfo.bIsMotorMoving = FALSE;}

	if (g_s4VMC8178AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = TRUE;}
	else						{stMotorInfo.bIsMotorOpen = FALSE;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stVMC8178AF_MotorInfo)))
    {
        VMC8178AFDB("[VMC8178AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveVMC8178AF(unsigned long a_u4Position)
{

  unsigned short TempPosition;
	
	VMC8178AFDB("[VMC8178AF] moveVMC8178AF = %d \n", a_u4Position);
    if((a_u4Position > g_u4VMC8178AF_MACRO  ) || (a_u4Position < g_u4VMC8178AF_INF))
    {
        VMC8178AFDB("[VMC8178AF] out of range \n");
        return -EINVAL;
    }

TempPosition=(unsigned long)(PositionHigh-((PositionHigh-PositionLow)*a_u4Position)/1024);
a_u4Position=TempPosition;

	VMC8178AFDB("[VMC8178AF] moveVMC8178AF TempPosition= %d \n", TempPosition);
	
	if (g_s4VMC8178AF_Opened == 1)
	{
		unsigned short InitPos;
             unsigned short DeviceId;
         
	       s4VMC8178AF_Read_ID(&DeviceId);

              VMC8178AFDB("[VMC8178AF] DeviceId 0x%x \n", DeviceId);

			
		if(s4VMC8178AF_ReadReg(&InitPos) == 0)
		{
			VMC8178AFDB("[VMC8178AF] Init Pos %6d \n", InitPos);
		
			g_u4CurrPosition = (unsigned long) ((PositionHigh-InitPos)*1024/(PositionHigh-PositionLow));
		}
		else
		{
			g_u4CurrPosition = 0;
		}
		
		g_s4VMC8178AF_Opened = 2;
	}
	VMC8178AFDB("[VMC8178AF] g_u4CurrPosition = %d \n", g_u4CurrPosition);

	if      (g_u4CurrPosition < a_u4Position)	{g_i4Dir = 1;}
	else if (g_u4CurrPosition > a_u4Position)	{g_i4Dir = -1;}
	else										{return 0;}
	VMC8178AFDB("[VMC8178AF] g_i4Dir = %d \n", g_i4Dir);

	if (1)
	{
		g_i4Position = (long)g_u4CurrPosition;
		g_u4TargetPosition = a_u4Position;

		if (g_i4Dir == 1)
		{
			//if ((g_u4TargetPosition - g_u4CurrPosition)<60)
			{		
				g_i4MotorStatus = 0;
				if(s4VMC8178AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					VMC8178AFDB("[VMC8178AF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;
			//}
		}
		else if (g_i4Dir == -1)
		{
			//if ((g_u4CurrPosition - g_u4TargetPosition)<60)
			{
				g_i4MotorStatus = 0;		
				if(s4VMC8178AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					VMC8178AFDB("[VMC8178AF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;		
			//}
		}
	}
	else
	{
	g_i4Position = (long)g_u4CurrPosition;
	g_u4TargetPosition = a_u4Position;
	g_i4MotorStatus = 1;
	}

    return 0;
}

inline static int setVMC8178AFInf(unsigned long a_u4Position)
{
	g_u4VMC8178AF_INF = a_u4Position;

	VMC8178AFDB("[VMC8178AF] setVMC8178AFInf \n");
	
	return 0;
}

inline static int setVMC8178AFMacro(unsigned long a_u4Position)
{
	g_u4VMC8178AF_MACRO = a_u4Position;

	VMC8178AFDB("[VMC8178AF] setVMC8178AFMacro \n");
	
	return 0;	
}

////////////////////////////////////////////////////////////////
static long VMC8178AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case VMC8178AFIOC_G_MOTORINFO :
            i4RetValue = getVMC8178AFInfo((__user stVMC8178AF_MotorInfo *)(a_u4Param));
        break;

        case VMC8178AFIOC_T_MOVETO :
            i4RetValue = moveVMC8178AF(a_u4Param);
        break;
 
 		case VMC8178AFIOC_T_SETINFPOS :
			 i4RetValue = setVMC8178AFInf(a_u4Param);
		break;

 		case VMC8178AFIOC_T_SETMACROPOS :
			 i4RetValue = setVMC8178AFMacro(a_u4Param);
		break;
		
        default :
      	     VMC8178AFDB("[VMC8178AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}
/*
static void VMC8178AF_WORK(struct work_struct *work)
{
    g_i4Position += (25 * g_i4Dir);

    if ((g_i4Dir == 1) && (g_i4Position >= (long)g_u4TargetPosition))
	{
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0;
    }

    if ((g_i4Dir == -1) && (g_i4Position <= (long)g_u4TargetPosition))
    {
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0; 		
    }
	
    if(s4VMC8178AF_WriteReg((unsigned short)g_i4Position) == 0)
    {
        g_u4CurrPosition = (unsigned long)g_i4Position;
    }
    else
    {
        VMC8178AFDB("[VMC8178AF] set I2C failed when moving the motor \n");
        g_i4MotorStatus = -1;
    }
}

static void VMC8178AF_ISR(UINT16 a_input)
{
	if (g_i4MotorStatus == 1)
	{	
		schedule_work(&g_stWork);		
	}
}
*/
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int VMC8178AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{

int i;
spin_lock(&g_VMC8178AF_SpinLock);
g_pstVMC8178AF_I2Cclient->addr=0x92>>1;
spin_unlock(&g_VMC8178AF_SpinLock);

char puSendCtrlCmd1[2] = { 27 , VASSTEK[27]};  
char puSendCtrlCmd2[2] = { 11 , VASSTEK[11]};  
char puSendCtrlCmd3[2] = { 12 , VASSTEK[12]};  
char puSendCtrlCmd4[2] = { 0x1f , 0xa8};  
char puSendCtrlCmd[2];
  //  spin_lock(&g_VMC8178AF_SpinLock);
	VMC8178AFDB("[VMC8178AF]opened \n");

    if(g_s4VMC8178AF_Opened)
    {
     //   spin_unlock(&g_VMC8178AF_SpinLock);
        VMC8178AFDB("[VMC8178AF] the device is opened \n");
        return -EBUSY;
    }


      // Init VMC8178
	i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd1, 2);
  i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd2, 2);
	i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd3, 2);
	 for(i=28;i<=168;i++)
	 {
	 	    puSendCtrlCmd[0]=i;
	 	    puSendCtrlCmd[1]=VASSTEK[i];
        i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd, 2);
   }
// Servo On , set mode
	i2c_master_send(g_pstVMC8178AF_I2Cclient, puSendCtrlCmd4, 2);
	
    GetBoundary();
    VMC8178AFDB("[VMC8178AF] g_u4VMC8178AF_INF = %d,g_u4VMC8178AF_MACRO=%d \n", PositionHigh,PositionLow);

  s4VMC8178AF_WriteReg((PositionLow + PositionHigh)/2);
	

   spin_lock(&g_VMC8178AF_SpinLock);
	
    g_s4VMC8178AF_Opened = 1;
		
    spin_unlock(&g_VMC8178AF_SpinLock);

	// --- Config Interrupt ---
	//g_GPTconfig.num = XGPT7;
	//g_GPTconfig.mode = XGPT_REPEAT;
	//g_GPTconfig.clkDiv = XGPT_CLK_DIV_1;//32K
	//g_GPTconfig.u4Compare = 32*2; // 2ms
	//g_GPTconfig.bIrqEnable = TRUE;
	
	//XGPT_Reset(g_GPTconfig.num);	
	//XGPT_Init(g_GPTconfig.num, VMC8178AF_ISR);

	//if (XGPT_Config(g_GPTconfig) == FALSE)
	//{
        //VMC8178AFDB("[VMC8178AF] ISR Config Fail\n");	
	//	return -EPERM;
	//}

	//XGPT_Start(g_GPTconfig.num);		

	// --- WorkQueue ---	
	//INIT_WORK(&g_stWork,VMC8178AF_WORK);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int VMC8178AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
	unsigned int cnt = 0;
	VMC8178AFDB("[VMC8178AF]release \n");

	if (g_s4VMC8178AF_Opened)
	{
	#if 0
		moveVMC8178AF(g_u4VMC8178AF_INF);
	#else
		moveVMC8178AF(200);
		msleep(20);
		moveVMC8178AF(100);
		msleep(20);
		moveVMC8178AF(50);
		msleep(10);
		
	#endif

		while(g_i4MotorStatus)
		{
			msleep(1);
			cnt++;
			VMC8178AFDB("cnt0 = %d\n",cnt);
			if (cnt>200)	{break;}
		}
		VMC8178AFDB("cnt1 = %d\n",cnt);
    	spin_lock(&g_VMC8178AF_SpinLock);

	    g_s4VMC8178AF_Opened = 0;

    	spin_unlock(&g_VMC8178AF_SpinLock);

    	//hwPowerDown(CAMERA_POWER_VCAM_A,"kd_camera_hw");

		//XGPT_Stop(g_GPTconfig.num);
	}

    return 0;
}

static const struct file_operations g_stVMC8178AF_fops = 
{
    .owner = THIS_MODULE,
    .open = VMC8178AF_Open,
    .release = VMC8178AF_Release,
    .unlocked_ioctl = VMC8178AF_Ioctl
};

inline static int Register_VMC8178AF_CharDrv(void)
{

#if 1
    struct device* vcm_device = NULL;
    VMC8178AFDB("[VMC8178AF] Register_VMC8178AF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_VMC8178AF_devno, 0, 1,VMC8178AF_DRVNAME) )
    {
        VMC8178AFDB("[VMC8178AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pVMC8178AF_CharDrv = cdev_alloc();

    if(NULL == g_pVMC8178AF_CharDrv)
    {
        unregister_chrdev_region(g_VMC8178AF_devno, 1);

        VMC8178AFDB("[VMC8178AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pVMC8178AF_CharDrv, &g_stVMC8178AF_fops);

    g_pVMC8178AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pVMC8178AF_CharDrv, g_VMC8178AF_devno, 1))
    {
        VMC8178AFDB("[VMC8178AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_VMC8178AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv0");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        VMC8178AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_VMC8178AF_devno, NULL, VMC8178AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    VMC8178AFDB("[VMC8178AF] Register_VMC8178AF_CharDrv - End\n");   
#endif	
    return 0;
}

inline static void Unregister_VMC8178AF_CharDrv(void)
{
    VMC8178AFDB("[VMC8178AF] Unregister_VMC8178AF_CharDrv - Start\n");
    //Release char driver
    cdev_del(g_pVMC8178AF_CharDrv);

    unregister_chrdev_region(g_VMC8178AF_devno, 1);
    
    device_destroy(actuator_class, g_VMC8178AF_devno);

    class_destroy(actuator_class);
    VMC8178AFDB("[VMC8178AF] Unregister_VMC8178AF_CharDrv - End\n");    
}

//////////////////////////////////////////////////////////////////////
/* Kirby: remove old-style driver
static int VMC8178AF_i2c_attach(struct i2c_adapter * a_pstAdapter);
static int VMC8178AF_i2c_detach_client(struct i2c_client * a_pstClient);
static struct i2c_driver VMC8178AF_i2c_driver = {
    .driver = {
    .name = VMC8178AF_DRVNAME,
    },
    //.attach_adapter = VMC8178AF_i2c_attach,
    //.detach_client = VMC8178AF_i2c_detach_client
};*/

/* Kirby: add new-style driver { */
//static int VMC8178AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int VMC8178AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int VMC8178AF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id VMC8178AF_i2c_id[] = {{VMC8178AF_DRVNAME,0},{}};   
//static unsigned short force[] = {IMG_SENSOR_I2C_GROUP_ID, VMC8178AF_VCM_WRITE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
//static const unsigned short * const forces[] = { force, NULL };              
//static struct i2c_client_address_data addr_data = { .forces = forces,}; 
struct i2c_driver VMC8178AF_i2c_driver = {                       
    .probe = VMC8178AF_i2c_probe,                                   
    .remove = VMC8178AF_i2c_remove,                           
    .driver.name = VMC8178AF_DRVNAME,                 
    .id_table = VMC8178AF_i2c_id,                             
};  

#if 0 
static int VMC8178AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, VMC8178AF_DRVNAME);                                                         
    return 0;                                                                                       
}      
#endif 
static int VMC8178AF_i2c_remove(struct i2c_client *client) {
    return 0;
}
/* Kirby: } */


/* Kirby: remove old-style driver
int VMC8178AF_i2c_foundproc(struct i2c_adapter * a_pstAdapter, int a_i4Address, int a_i4Kind)
*/
/* Kirby: add new-style driver {*/
static int VMC8178AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
/* Kirby: } */
{
    int i4RetValue = 0;

    VMC8178AFDB("[VMC8178AF] Attach I2C \n");

    /* Kirby: remove old-style driver
    //Check I2C driver capability
    if (!i2c_check_functionality(a_pstAdapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        VMC8178AFDB("[VMC8178AF] I2C port cannot support the format \n");
        return -EPERM;
    }

    if (!(g_pstVMC8178AF_I2Cclient = kzalloc(sizeof(struct i2c_client), GFP_KERNEL)))
    {
        return -ENOMEM;
    }

    g_pstVMC8178AF_I2Cclient->addr = a_i4Address;
    g_pstVMC8178AF_I2Cclient->adapter = a_pstAdapter;
    g_pstVMC8178AF_I2Cclient->driver = &VMC8178AF_i2c_driver;
    g_pstVMC8178AF_I2Cclient->flags = 0;

    strncpy(g_pstVMC8178AF_I2Cclient->name, VMC8178AF_DRVNAME, I2C_NAME_SIZE);

    if(i2c_attach_client(g_pstVMC8178AF_I2Cclient))
    {
        kfree(g_pstVMC8178AF_I2Cclient);
    }
    */
    /* Kirby: add new-style driver { */
    g_pstVMC8178AF_I2Cclient = client;
    
    g_pstVMC8178AF_I2Cclient->addr = g_pstVMC8178AF_I2Cclient->addr >> 1;
    
    /* Kirby: } */

    //Register char driver
    i4RetValue = Register_VMC8178AF_CharDrv();

    if(i4RetValue){

        VMC8178AFDB("[VMC8178AF] register char device failed!\n");

        /* Kirby: remove old-style driver
        kfree(g_pstVMC8178AF_I2Cclient); */

        return i4RetValue;
    }

    spin_lock_init(&g_VMC8178AF_SpinLock);

    VMC8178AFDB("[VMC8178AF] Attached!! \n");

    return 0;
}

/* Kirby: remove old-style driver
static int VMC8178AF_i2c_attach(struct i2c_adapter * a_pstAdapter)
{

    if(a_pstAdapter->id == 0)
    {
    	 return i2c_probe(a_pstAdapter, &g_stVMC8178AF_Addr_data ,  VMC8178AF_i2c_foundproc);
    }

    return -1;

}

static int VMC8178AF_i2c_detach_client(struct i2c_client * a_pstClient)
{
    int i4RetValue = 0;

    Unregister_VMC8178AF_CharDrv();

    //detach client
    i4RetValue = i2c_detach_client(a_pstClient);
    if(i4RetValue)
    {
        dev_err(&a_pstClient->dev, "Client deregistration failed, client not detached.\n");
        return i4RetValue;
    }

    kfree(i2c_get_clientdata(a_pstClient));

    return 0;
}*/

static int VMC8178AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&VMC8178AF_i2c_driver);
}

static int VMC8178AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&VMC8178AF_i2c_driver);
    return 0;
}

static int VMC8178AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
//    int retVal = 0;
//    retVal = hwPowerDown(MT6516_POWER_VCAM_A,VMC8178AF_DRVNAME);

    return 0;
}

static int VMC8178AF_resume(struct platform_device *pdev)
{
/*
    if(TRUE != hwPowerOn(MT6516_POWER_VCAM_A, VOL_2800,VMC8178AF_DRVNAME))
    {
        VMC8178AFDB("[VMC8178AF] failed to resume VMC8178AF\n");
        return -EIO;
    }
*/
    return 0;
}

// platform structure
static struct platform_driver g_stVMC8178AF_Driver = {
    .probe		= VMC8178AF_probe,
    .remove	= VMC8178AF_remove,
    .suspend	= VMC8178AF_suspend,
    .resume	= VMC8178AF_resume,
    .driver		= {
        .name	= "lens_actuator",
        .owner	= THIS_MODULE,
    }
};

static int __init VMC8178AF_i2C_init(void)
{
	 i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);
	 VMC8178AFDB("register VMC8178AF driver\n");
    if(platform_driver_register(&g_stVMC8178AF_Driver)){
        VMC8178AFDB("failed to register VMC8178AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit VMC8178AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stVMC8178AF_Driver);
}

module_init(VMC8178AF_i2C_init);
module_exit(VMC8178AF_i2C_exit);

MODULE_DESCRIPTION("VMC8178AF lens module driver");
MODULE_AUTHOR("Gipi Lin <Gipi.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


