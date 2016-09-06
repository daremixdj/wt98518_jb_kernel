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

#ifndef __PCMFMT_COM_H__
#define __PCMFMT_COM_H__

#include "wmatypes.h"

typedef enum _PCMData {
    PCMDataPCM        = 0,
    PCMDataIEEE_FLOAT = 1,
} PCMData;

#pragma pack (push)
#pragma pack () // default
typedef struct _PCMFormat {
    U32 nSamplesPerSec;               
    U32 nChannels;
    U32 nChannelMask;
    U32 nValidBitsPerSample;
    U32 cbPCMContainerSize;
    PCMData pcmData;
} PCMFormat;
#pragma pack (pop)

#endif//__PCMFMT_COM_H__

#ifndef __PCMFMT_H__
#define __PCMFMT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WAVEFORMATEX_
STATIC_DECLARE void WaveFormatEx2PCMFormat(WAVEFORMATEX* wfx, PCMFormat* pFormat);
STATIC_DECLARE void PCMFormat2WaveFormatEx(PCMFormat* pFormat, WAVEFORMATEX* wfx);
STATIC_DECLARE Bool ValidateWaveFormat(WAVEFORMATEX* wfx);
STATIC_DECLARE Bool ValidateUncompressedFormat(WAVEFORMATEX* wfx);

#ifdef _WAVEFORMATEXTENSIBLE_
STATIC_DECLARE void PCMFormat2WaveFormatExtensible(PCMFormat* pFormat, WAVEFORMATEXTENSIBLE* wfx);
#endif//_WAVEFORMATEXTENSIBLE_
#endif//_WAVEFORMATEX_

#ifdef GUID_DEFINED
STATIC_DECLARE void FormatTag2GUID(U16 wFormatTag, GUID* pGUID);
#endif

STATIC_DECLARE Bool NeedExtensible(U32 nBitsPerSample, U32 nChannels, U32 nChannelMask);
STATIC_DECLARE Bool PCMFormatEquivalent(PCMFormat*, PCMFormat*);
STATIC_DECLARE U32 DefaultChannelMask(U32 nChannels);

#ifdef __cplusplus
}
#endif

#endif//__PCMFMT_H__

