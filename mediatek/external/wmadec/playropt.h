/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
#ifndef __PLAYROPT_H__
#define __PLAYROPT_H__

// **************************************************************************
// Player Options
// **************************************************************************
#define PLAYOPT_HALFTRANSFORM      0x0002
#define PLAYOPT_PAD2XTRANSFORM     0x0008
#define PLAYOPT_DYNAMICRANGECOMPR  0x0080
#define PLAYOPT_LTRT               0x0100
// defines for WMAVoice
// in this CPU_RESOURCE field, 0 and 4 both equate to a 206 MHz StrongARM level of cpu performance.  
// Some future release may implement auomatic adaption for 0 but not for 4.
// 1 is suitable for an ARM7 or ARM9 implementation that runs at about 70 MHz.
// other CPU's should probably pass 0 but can try other values.
// the codec may apply higher levels of post processing for higher levels of this value which will result in higher quality.
// However, raising the number too far will cause the codec to try too complex post processing and result in stuttering audio.
#define PLAYOPT_CPU_RESOURCE_MASK   0x000F0000
#define PLAYOPT_CPU_RESOURCE_AUTO   0
#define PLAYOPT_CPU_RESOURCE(a) ((a)<<16)
#define PLAYOPT_CPU_RESOURCE_LOW    PLAYOPT_CPU_RESOURCE(2)
#define PLAYOPT_CPU_RESOURCE_NORMAL PLAYOPT_CPU_RESOURCE(8)

// **************************************************************************
// Dynamic Range Control settings
// **************************************************************************
#define WMA_DRC_HIGH 0
#define WMA_DRC_MED 1
#define WMA_DRC_LOW 2

// **************************************************************************
// Player Info Structure
// **************************************************************************

typedef struct _WMAPlayerInfo
{
    WMA_U16 nPlayerOpt;        // Bit fields based on above defines
    WMA_I32 *rgiMixDownMatrix; // Can be null to invoke defaults
    WMA_I32 iPeakAmplitudeRef;
    WMA_I32 iRmsAmplitudeRef;
    WMA_I32 iPeakAmplitudeTarget;
    WMA_I32 iRmsAmplitudeTarget;
    WMA_I16 nDRCSetting;       // Dynamic range control setting
} WMAPlayerInfo;

#endif//__PLAYROPT_H__
