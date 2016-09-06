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

    wmatypes.h

Abstract:

    Processor-specific data types.

Author:

    Kazuhito Koishida (kazukoi)       Jan 31, 2003

Revision History:

*************************************************************************/

#ifndef __WMATYPES_H
#define __WMATYPES_H

// ***************************************************
//           Platform-Specific typedefs
// ***************************************************
#ifndef DISABLE_OPT	// If this is defined, use the ANSI build

#ifdef __QNX__
#undef _M_IX86
#endif

#if defined(_M_IX86) && !defined(INTEGER_ENCODER)
//#ifdef __QNX__
//#define WMA_TARGET_QNX_X86
//#else
#define WMA_TARGET_X86
//#endif
#endif

#ifdef _M_AMD64
  #ifndef WMA_TARGET_WIN64
    #define WMA_TARGET_WIN64
  #endif
  #ifndef DISABLE_INTELFFT
    #define DISABLE_INTELFFT
  #endif 
#endif

#ifdef _M_IA64
  #ifndef WMA_TARGET_WIN64
    #define WMA_TARGET_WIN64
  #endif
  #ifndef DISABLE_INTELFFT
    #define DISABLE_INTELFFT
  #endif 
#endif

#ifdef _MIPS_
#define WMA_TARGET_MIPS
#endif

#if defined(_SH3_) && !defined(_SH4_)
#define WMA_TARGET_SH3
#endif

#ifdef _SH4_
#define WMA_TARGET_SH4
#endif

#ifdef _Embedded_x86
#define WMA_TARGET_Embedded_x86
#endif

#ifdef S_SUNOS5
#define WMA_TARGET_S_SUNOS5
#endif

#ifdef _ARM_
#define WMA_TARGET_ARM
#endif

#ifdef _ARM_FP_
#define WMA_TARGET_ARM
#endif

// It's often nice to be able to compare the output between optimized versions
// and the ANSI version, to verify that the optimizations are in sync. Alas,
// some optimizations sacrifice reproducibility for speed. The following #define allows
// us to disable those optimizations to verify the main algorithms.
//#define WMA_ANSI_COMPATIBILITY_MODE

#endif	// !DISABLE_OPT

#if defined(WMA_TARGET_WIN64)
#include "wmatypes_win64.h"
#elif defined(WMA_TARGET_X86)
#include "wmatypes_x86.h"
#elif defined(WMA_TARGET_QNX_X86)
#include "wmatypes_qnx_x86.h"
#elif defined(WMA_TARGET_MIPS)
#include "wmatypes_mips.h"
#elif defined(WMA_TARGET_SH3)
#include "wmatypes_sh3.h"
#elif defined(WMA_TARGET_SH4)
#include "wmatypes_sh4.h"
#elif defined(WMA_TARGET_ARM)
#include "wmatypes_arm.h"
#elif defined(WMA_TARGET_Embedded_x86)
#include "wmatypes_embedded_x86.h"
#elif defined(WMA_TARGET_S_SUNOS5)
#include "wmatypes_s_sunos5.h"
#elif defined(WMA_TARGET_MACOS)
#include "wmatypes_macos.h"
#else
#define WMA_TARGET_ANSI
#include "wmatypes_ansi.h"
#endif // platform specific #ifdefs


// ***************************************************
//            default fundamental typedefs
// ***************************************************
#ifndef PLATFORM_SPECIFIC_U64
typedef unsigned __int64 U64;
#endif
#ifndef PLATFORM_SPECIFIC_I64
typedef __int64 I64;
#endif

#ifndef PLATFORM_SPECIFIC_U32
#ifdef _ARM_SP_
// resolve U32 redefinition errror
#define PLATFORM_SPECIFIC_U32
#endif
typedef unsigned long int U32;
#define U32_MAX ((U32)0xffffffffu)
#endif
#ifndef PLATFORM_SPECIFIC_I32
#ifdef _ARM_SP_
// resolve I32 redefinition errror
#define PLATFORM_SPECIFIC_I32
#endif
typedef long int I32;
#define I32_MIN ((I32)0x80000000)
#define I32_MAX ((I32)0x7fffffff)
#endif

#ifndef PLATFORM_SPECIFIC_U16
#ifdef _ARM_SP_
// resolve U16 redefinition errror
#define PLATFORM_SPECIFIC_U16
#endif
typedef unsigned short U16;
#define U16_MAX ((I32)0x0000ffff)
#endif
#ifndef PLATFORM_SPECIFIC_I16
#ifdef _ARM_SP_
// resolve I16 redefinition errror
#define PLATFORM_SPECIFIC_I16
#endif
typedef short I16;
#define I16_MIN ((I32)0xffff8000)
#define I16_MAX ((I32)0x00007fff)
#endif

#ifndef PLATFORM_SPECIFIC_U8
#ifdef _ARM_SP_
// resolve U8 redefinition errror
#define PLATFORM_SPECIFIC_U8
#endif
typedef unsigned char U8;
#define U8_MAX ((I32)0x000000ff)
#endif
#ifndef PLATFORM_SPECIFIC_I8
#ifdef _ARM_SP_
// resolve I8 redefinition errror
#define PLATFORM_SPECIFIC_I8
#endif
typedef signed char I8;
#define I8_MIN ((I32)0xffffff80)
#define I8_MAX ((I32)0x0000007f)
#endif

#ifndef PLATFORM_SPECIFIC_F32
typedef float F32;
#endif
#ifndef PLATFORM_SPECIFIC_F64
typedef double F64;
#endif

// the following "UP" types are for using n-bit or native type, in case
// using native types might be faster.
#ifndef PLATFORM_SPECIFIC_I8UP
typedef int I8UP;
#endif

#ifndef PLATFORM_SPECIFIC_U8UP
typedef unsigned U8UP;
#endif

#ifndef PLATFORM_SPECIFIC_I16UP
typedef int I16UP;
#endif

#ifndef PLATFORM_SPECIFIC_U16UP
typedef unsigned U16UP;
#endif

#ifndef PLATFORM_SPECIFIC_I32UP
typedef int I32UP;
#endif

#ifndef PLATFORM_SPECIFIC_U32UP
typedef unsigned U32UP;
#endif

#ifndef PLATFORM_SPECIFIC_I64UP
#ifdef _WIN32
typedef __int64 I64UP;
#else
typedef long long I64UP;
#endif
#endif

#ifndef PLATFORM_SPECIFIC_U64UP
#ifdef _WIN32
typedef unsigned __int64 U64UP;
#else
typedef unsigned long long U64UP;
#endif
#endif

#ifndef PLATFORM_SPECIFIC_F32UP
typedef float F32UP;
#endif
#ifndef PLATFORM_SPECIFIC_F64UP
typedef double F64UP;
#endif


typedef double  Double;
typedef float   Float;
typedef void    Void;
typedef U32     UInt;
typedef I32     Int;
typedef I32     Bool; // In Win32, BOOL is an "int" == 4 bytes. Keep it this way to reduce problems.

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

// define basic types
#ifdef _WIN32
typedef __int64 WMA_I64;
#else
typedef long long       WMA_I64;
#endif
#ifndef WMA_U32
typedef unsigned long WMA_U32;
#endif

//typedef unsigned long   WMA_U32;
typedef long            WMA_I32;
typedef unsigned short  WMA_U16;
typedef short           WMA_I16;
typedef unsigned char   WMA_U8;
typedef WMA_I32         WMA_Bool;

typedef int WMARESULT;

#endif  // __WMATYPES_H
