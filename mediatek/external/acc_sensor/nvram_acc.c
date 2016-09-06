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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
	 
#include "../../kernel/include/linux/sensors_io.h"
#include "libnvram.h"
#include "CFG_GS_File.h"
#include "Custom_NvRam_LID.h"
#include <linux/ioctl.h>

int main(void) 
{
	int i, ret;
	int err;
	NVRAM_LOG("NVRAM acc enter !\n");
	printf("NVRAM acc enter !,,lid=%d\n",AP_CFG_RDCL_HWMON_ACC_WTCH_LID);

	int file_lid = AP_CFG_RDCL_HWMON_ACC_WTCH_LID;    
    int rec_size;
    int rec_num, fd_gs, res;
	
	F_ID  fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);

	NVRAM_HWMON_ACC_WTCH_STRUCT hwmonGs =
	{	 
		{0, 0, 0, 0},
	};	   
  
	if(fd.iFileDesc < 0) 
	{
		NVRAM_LOG("nvram open = %d\n", fd.iFileDesc);
		return fd.iFileDesc;
	}
	
	res = read(fd.iFileDesc, &hwmonGs, rec_size*rec_num); 
	if(res < 0)
	{
		NVRAM_LOG("nvram read = %d(%s)\n", errno, strerror(errno));
	}
	else
	{
		fd_gs = open("/dev/gsensor", O_RDONLY);
		if(fd_gs<0)
		{
			NVRAM_LOG("/dev/als_gs open = %d\n", fd.iFileDesc);
		}
		else
		{
			if(err = ioctl(fd_gs, GSENSOR_IOCTL_SET_CALI_NEW, &hwmonGs)) 
			{
				 printf("set cali new: %d(%s)\n", err, strerror(errno));
				 return err;
			}
		}
		close(fd_gs);
	}
	NVRAM_LOG("NVRAM ACC: valid=%d,x=%d,y=%d,z=%d\n",hwmonGs.valid,hwmonGs.x,hwmonGs.y,hwmonGs.z);
	NVM_CloseFileDesc(fd); 
	NVRAM_LOG("NVRAM ACC exits!\n");
	exit(EXIT_SUCCESS);
}

