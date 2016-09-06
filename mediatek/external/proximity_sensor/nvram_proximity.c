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

#ifdef CUSTOM_KERNEL_ALSPS
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <linux/sensors_io.h>
//#include "../../kernel/include/linux/sensors_io.h"

#include "libnvram.h"
#include "CFG_PS_File.h"
#include "CFG_PS_Default.h"
#include "Custom_NvRam_LID.h"



int main(void) 
{
	int i, ret;
	int err;
	NVRAM_LOG("NVRAM proximity enter !\n");

	int file_lid = AP_CFG_RDCL_HWMON_PS_LID;    
        int rec_size;
        int rec_num,fd_ps;
	//int res, fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	int res;
	F_ID  fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	NVRAM_HWMON_PS_STRUCT hwmonPs =
		{	 
			{0,  0, 0},
		};	  
		
		if(fd.iFileDesc < 0) // modified by fangliang [fd]
		{
			NVRAM_LOG("nvram open = %d\n", fd);
			return fd.iFileDesc;// fangliang modified [fd]
		}
		
		res = read(fd.iFileDesc, &hwmonPs , rec_size*rec_num);// fangliang   ifile_lid
		//printf("fangliang: the nvram data_ps close = %d,far_away = %d  res=%d\n",hwmonPs.close,hwmonPs.far_away);
		if(res < 0)
		{
			NVRAM_LOG("nvram read = %d(%s)\n", errno, strerror(errno));
		}
		else
		{
			fd_ps = open("/dev/als_ps", O_RDONLY);
			if(fd_ps<0)
				{
					NVRAM_LOG("/dev/als_ps open = %d\n", fd.iFileDesc);
				}
			else
				{
					if(err = ioctl(fd_ps, ALSPS_SET_PS_CALI, &hwmonPs))
						NVRAM_LOG("set_ps_cali err: %d\n", err);
				}
			close(fd_ps);
		}

	NVRAM_LOG("the nvram data_ps close = %d,far_away = %d\n",hwmonPs.close,hwmonPs.far_away);
	NVM_CloseFileDesc(fd); 
	NVRAM_LOG("NVRAM proximity exits !\n");
	exit(EXIT_SUCCESS);
}

#endif 
