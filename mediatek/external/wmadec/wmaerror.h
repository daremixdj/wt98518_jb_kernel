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
#ifndef __WMAERROR_H__
#define __WMAERROR_H__

#include "wmatyps.h"

// SUCCESS codes
static const WMARESULT WMA_OK               = 0x00000000;
static const WMARESULT WMA_S_FALSE          = 0x00000001;
static const WMARESULT WMA_S_BUFUNDERFLOW   = 0x00000002;
static const WMARESULT WMA_S_NEWPACKET      = 0x00000003;
static const WMARESULT WMA_S_NO_MORE_FRAME  = 0x00000004;
static const WMARESULT WMA_S_NO_MORE_SRCDATA= 0x00000005;
static const WMARESULT WMA_S_LOSTPACKET     = 0x00000006;

// ERROR codes
static const WMARESULT WMA_E_FAIL           = 0x80004005;
static const WMARESULT WMA_E_OUTOFMEMORY    = 0x8007000E;
static const WMARESULT WMA_E_INVALIDARG     = 0x80070057;
static const WMARESULT WMA_E_NOTSUPPORTED   = 0x80040000;
static const WMARESULT WMA_E_BROKEN_FRAME   = 0x80040002;
static const WMARESULT WMA_E_BUFFEROVERFLOW = 0x80040003;
static const WMARESULT WMA_E_ONHOLD         = 0x80040004; 
static const WMARESULT WMA_E_NO_MORE_SRCDATA= 0x80040005;
static const WMARESULT WMA_E_WRONGSTATE     = 0x8004000A;
static const WMARESULT WMA_E_NOMOREINPUT    = 0x8004000B; // Streaming mode: absolutely no more data (returned via GetMoreData callback)

#define WMA_SUCCEEDED(Status) ((WMARESULT)(Status) >= 0)
#define WMA_FAILED(Status) ((WMARESULT)(Status)<0)

#endif//__WMAERROR_H__
