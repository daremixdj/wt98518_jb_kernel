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
/*************************************************************************

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    wmatypes_ansi.h

Abstract:

    ANSI-C data types.

Author:

    Kazuhito Koishida (kazukoi)       Jan 31, 2003

Revision History:


*************************************************************************/

#ifndef __WMATYPES_ANSI_H
#define __WMATYPES_ANSI_H

// Do the fundamental typedefs: {U|I}{8|16|32|64}
#ifdef NOINT64 

#include "int64.h"
#define PLATFORM_SPECIFIC_U64
typedef UINT64 U64;
#define PLATFORM_SPECIFIC_I64?
typedef INT64 I64;

#else
#ifdef __QNX__

#define PLATFORM_SPECIFIC_U64
typedef unsigned long long int U64;
#define PLATFORM_SPECIFIC_I64
typedef long long int I64;

#else
#ifdef macintosh

#define PLATFORM_SPECIFIC_U64
typedef unsigned long long U64;
#define PLATFORM_SPECIFIC_I64
typedef long long I64;

#else

#define PLATFORM_SPECIFIC_U64
#define PLATFORM_SPECIFIC_I64
#ifdef _WIN32
typedef unsigned __int64 U64;
typedef __int64 I64;
#else
typedef unsigned long long U64;
typedef long long I64;
#endif

// #define PLATFORM_SPECIFIC_U64
// typedef unsigned long long U64;
// #define PLATFORM_SPECIFIC_I64
// typedef long long I64;

#endif
#endif
#endif //NOINT64

#endif  //_WMATYPES_ANSI_H
