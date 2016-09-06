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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_s5k3h2yxmipiraw.h"
#include "camera_info_s5k3h2yxmipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
    }
    }},
    ISPCcmPoly22:{
        71275,    // i4R_AVG
        16046,    // i4R_STD
        102075,    // i4B_AVG
        24099,    // i4B_STD
        { // i4P00[9]
            5312500, -2762500, 7500, -905000, 4047500, -577500, -80000, -2250000, 4890000
        },
        { // i4P10[9]
            2333686, -2685379, 350559, -205446, -273217, 473894, 34653, 314530, -330362
        },
        { // i4P01[9]
            1841234, -2168741, 338976, -363640, -387740, 753649, -33228, -324909, 381626
        },
        { // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        { // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }        
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1260,   // u4MinGain, 1024 base =  1x
            8192,  // u4MaxGain, 16x
            162,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8
            27,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            18,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            27,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
         },
         // rHistConfig
        {
            2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {86, 108, 128, 148, 170},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableStrobeThres
            47,    // u4AETarget
            0,    // u4StrobeAETarget

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -27,    // i4BVOffset delta BV = value/10 
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            5,                 // u4VideoFlareThres
            2,                 // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            8,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            8,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75                 // u4FlatnessStrength
         }
    },

    // AWB NVRAM
    {
    	// AWB calibration data
    	{
    		// rUnitGain (unit gain: 1.0 = 512)
    		{
    			0,	// i4R
    			0,	// i4G
    			0	// i4B
    		},
    		// rGoldenGain (golden sample gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
    		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    		{
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                936,    // i4R
                512,    // i4G
                662    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -405,    // i4X
                -305    // i4Y
            },
            // A
            {
                -284,    // i4X
                -352    // i4Y
            },
            // TL84
            {
                -147,    // i4X
                -347    // i4Y
            },
            // CWF
            {
                -108,    // i4X
                -401    // i4Y
            },
            // DNP
            {
                128,    // i4X
                -318    // i4Y
            },
            // D65
            {
                128,    // i4X
                -318    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -416,    // i4X
                -291    // i4Y
            },
            // A
            {
                -296,    // i4X
                -342    // i4Y
            },
            // TL84
            {
                -159,    // i4X
                -342    // i4Y
            },
            // CWF
            {
                -122,    // i4X
                -397    // i4Y
            },
            // DNP
            {
                117,    // i4X
                -322    // i4Y
            },
            // D65
            {
                117,    // i4X
                -322    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                586,    // i4G
                1535    // i4B
            },
            // A 
            {
                561,    // i4R
                512,    // i4G
                1212    // i4B
            },
            // TL84 
            {
                671,    // i4R
                512,    // i4G
                1001    // i4B
            },
            // CWF 
            {
                761,    // i4R
                512,    // i4G
                1020    // i4B
            },
            // DNP 
            {
                936,    // i4R
                512,    // i4G
                662    // i4B
            },
            // D65 
            {
                936,    // i4R
                512,    // i4G
                662    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            }
        },
        // Rotation matrix parameter
        {
            2,    // i4RotationAngle
            256,    // i4Cos
            9    // i4Sin
        },
        // Daylight locus parameter
        {
            -135,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -209,    // i4RightBound
            -859,    // i4LeftBound
            -266,    // i4UpperBound
            -367    // i4LowerBound
            },
            // Warm fluorescent
            {
            -209,    // i4RightBound
            -859,    // i4LeftBound
            -367,    // i4UpperBound
            -487    // i4LowerBound
            },
            // Fluorescent
            {
            67,    // i4RightBound
            -209,    // i4LeftBound
            -254,    // i4UpperBound
            -369    // i4LowerBound
            },
            // CWF
            {
            67,    // i4RightBound
            -209,    // i4LeftBound
            -369,    // i4UpperBound
            -447    // i4LowerBound
            },
            // Daylight
            {
            142,    // i4RightBound
            67,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Shade
            {
            502,    // i4RightBound
            142,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            67,    // i4RightBound
            300,    // i4LeftBound
            -402,    // i4UpperBound
            -600    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            502,    // i4RightBound
            -859,    // i4LeftBound
            0,    // i4UpperBound
            -600    // i4LowerBound
            },
            // Daylight
            {
            167,    // i4RightBound
            67,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Cloudy daylight
            {
            267,    // i4RightBound
            92,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Shade
            {
            367,    // i4RightBound
            92,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Twilight
            {
            67,    // i4RightBound
            -93,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Fluorescent
            {
            167,    // i4RightBound
            -259,    // i4LeftBound
            -272,    // i4UpperBound
            -447    // i4LowerBound
            },
            // Warm fluorescent
            {
            -196,    // i4RightBound
            -396,    // i4LeftBound
            -272,    // i4UpperBound
            -447    // i4LowerBound
            },
            // Incandescent
            {
            -196,    // i4RightBound
            -396,    // i4LeftBound
            -242,    // i4UpperBound
            -402    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            936,    // i4R
            512,    // i4G
            662    // i4B
            },
            // Cloudy daylight
            {
            1016,    // i4R
            512,    // i4G
            606    // i4B
            },
            // Shade
            {
            1084,    // i4R
            512,    // i4G
            565    // i4B
            },
            // Twilight
            {
            790,    // i4R
            512,    // i4G
            794    // i4B
            },
            // Fluorescent
            {
            798,    // i4R
            512,    // i4G
            873    // i4B
            },
            // Warm fluorescent
            {
            576,    // i4R
            512,    // i4G
            1238    // i4B
            },
            // Incandescent
            {
            546,    // i4R
            512,    // i4G
            1179    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            10,    // i4SliderValue
            6024    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            10,    // i4SliderValue
            5243    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            936,    // i4R
            512,    // i4G
            662    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
		},
		// Preference gain: tungsten
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: warm fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: CWF
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: shade
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight fluorescent
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
    		}
    	},
    	// CCT estimation
    	{
    		// CCT
    		{
			    2300,	// i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -533,    // i4RotatedXCoordinate[0]
                -413,    // i4RotatedXCoordinate[1]
                -276,    // i4RotatedXCoordinate[2]
                0,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT)};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        default:
            break;
    }
    return 0;
}};  //  NSFeature


