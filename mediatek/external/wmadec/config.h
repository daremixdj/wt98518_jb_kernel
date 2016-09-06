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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else 
#define EXTERN_C extern 
#endif
#endif

#include "wmatyps.h"

// The XxxGetXxxXxx funstions below are used to enumerate supported parameter
// combinations.  ulIndex is a 0-based index, the funtions return FALSE if it
// is out of range.
//
// The XxxCheckXxxXxx functions return TRUE if the parameter combination is
// supported, FALSE otherwise.

typedef struct _WMAFormatInfo {
    // PCM params
    WMA_U32 nSamplesPerSec;
    WMA_U16 nChannels;
    WMA_U32 nChannelMask;
    WMA_U16 nValidBitsPerSample;
    
    // for system layers
    WMA_U16 nBlockAlign; // cbPacketLength
    WMA_U32 nAvgBytesPerSec;
    WMA_U32 nBufferSizeBits;
    
    // for decoder
    WMA_U16 wEncodeOptions;

    WMA_U32 nVersion;
    
    // for WMAUDIO2WAVEFORMAT (v2 only)
    WMA_U32 dwSuperBlockAlign;
    WMA_U32 nMaxSamplesPerPacket;
    
    // for auencEncode
    WMA_U32 nLookaheadSamples;

    // for auencInit
    WMA_U32 nFramesPerPacket;
    WMA_U32 nSamplesPerFrame;

    // informative
    WMA_U32 ulOfficialBitrate;
    WMA_Bool fQualityBasedVBR;
    WMA_U32 ulVBRQuality;

    // WMA Pro but Some frames are encoded by pure lossless codec. SPDIF unfriendly.
    // Though we can generate enforced MLLM content, we don't think it will be useful.
    // Therefore, we can think EnforcedUnifiedLLM is EnforcedUnfiedPLLM now.
    WMA_Bool  fEnforcedUnifiedLLM;     
} WMAFormatInfo;


typedef struct _CoefStream CoefStream;

typedef struct _WMAEncoderInfo
{
    WMA_U32   nBytePerSample;
    WMA_Bool  fVBR;          //fix quality mode
    float fltGlobalNmr;
    WMA_I32   cSampleMaxLatency;
    WMA_I32   m_tGivenBuffer; //in ms
    WMA_U16   wSourceFormatTag; // Used to decide whether to enter transcode mode or not
    WMA_U32   iSrcBitRate; // used in conjunction with wSourceFormatTag
    WMA_U16   nSrcChannels;// used in conjunction with wSourceFormatTag
    WMA_Bool  fTwoPass;         // Two-pass mode
    WMA_Bool  fSaveInputPCM;    // Two-pass VBR: if TRUE, save input PCM to file for use in 2nd pass
    WMA_Bool  fPeakConstrained; // Two-pass VBR only
    // ulRMax and ulBMax are valid only in two-pass VBR mode and only if fPeakConstraied is set
    WMA_U32   ulRMax; // bps
    WMA_U32   ulBMax; // milliseconds
} WMAEncoderInfo;

#ifdef __cplusplus
extern "C" {
#endif

   // Checks if the codec can encode this PCM at all (at at least some bitrate)
WMA_Bool CheckPCMParams(WMA_U32 ulSamplingRate, WMA_U16 ulChannels);
// Enumerate all of the channels / sampling combinations supported at any bitrate
STATIC_DECLARE WMA_U32 DefaultChannelMask(WMA_U32 nChannels);

const extern WMA_U32 g_ulFormatSetFlag;
#define FORMATSET_FLAG_EUTIL 0x01
#define FORMATSET_FLAG_DMO   0x02
#define FORMATSET_FLAG_LOWDELAY   0x04

//
// For the GetXxxXxx functions, the entire WMAFormatInfo is an output parameter.
// For CheckFormat, the WAVEFORMATEX stuff is input and the miscellaneous fields
// at the end are output.
//

WMA_U32 FormatCount(WMA_U32 ulVersion, WMA_Bool fTwoPass, WMA_Bool fVBR, WMA_Bool fAV);

WMA_Bool EnumFormats(WMA_U32 ulVersion, WMA_Bool fTwoPass, WMA_Bool fVBR, WMA_Bool fAV, WMA_U32 *pulIndex, WMAFormatInfo* pFormat);

WMA_Bool EnumByPCM(WMA_U32 ulVersion, WMA_Bool fTwoPass, WMA_Bool fVBR, WMA_Bool fAV, WMA_U32 *pulIndex,
               WMAFormatInfo* pFormat, WMA_U32 ulSamplingRate, WMA_U16 ulChannels,
               WMA_U32 ulChannelMask, WMA_U32 ulValidBitsPerSample);

// If you know the sampling rate / channels AND the approximate bitrate you
// want, call this to get the corresponding WMAFormatInfo parameters.
EXTERN_C WMA_Bool GetFormat(WMA_U32 ulSamplingRate, WMA_U16 ulChannels, 
                        WMA_U32 nValidBitsPerSample, WMA_U32 nChannelMask,
                        WMA_U32 ulVersion, WMA_Bool fTwoPass, WMA_Bool fVBR, WMA_Bool fAV,
                        WMA_U32 ulBitrate, WMA_U32 ulVBRQuality,
                        WMAFormatInfo* pFormat, WMAEncoderInfo* pEncInfo,
                        Bool *pfLowDelay);

// The DMO uses this to find the leaky bucket size for formats not coming from config.c
WMA_U32 wmaBufferSizeBits(WMA_U16 nFramesPerBuffer, 
                      WMA_U32 nFramesPerPacket,
                      WMA_U32 nSamplesPerFrame, 
                      WMA_U32 ulSamplingRate, 
                      WMA_U32 nAvgBytesPerSec,
                      WMA_U32 cbPacketLength);

EXTERN_C WMA_I32 wmaEncGetOutputBufferSizeInPackets (WMAFormatInfo *Format, 
                                                 WMA_Bool fVBR,
                                                 WMA_I32 cNumPacket);

WMARESULT wmaCfgSeedTwoPassVBR(const WMA_U32 dwAudioBitRate,
                               const WMA_U16 ulChannels,
                               const WMA_U32 ulVersion, 
                               WMA_U32 *pdwAudioQuality,
                               float *pfltGlobalNMR);

#ifdef __cplusplus
}
#endif


#endif //__CONFIG_H__
