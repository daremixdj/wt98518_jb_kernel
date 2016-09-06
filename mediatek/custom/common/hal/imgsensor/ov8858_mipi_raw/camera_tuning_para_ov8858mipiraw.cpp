#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8858mipiraw.h"
#include "camera_info_ov8858mipiraw.h"
#include "camera_custom_AEPlinetable.h"
//#include "camera_custom_tsf_tbl.h"
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
        75120,    // i4R_AVG
        16431,    // i4R_STD
        105400,    // i4B_AVG
        23164,    // i4B_STD
        {  // i4P00[9]
            5676000, -3088000, -22000, -848000, 4022000, -614000, -126000, -2448000, 5136000
        },
        {  // i4P10[9]
            2087985, -2368029, 262886, -180005, 51752, 128253, 72608, -31681, -31374
        },
        {  // i4P01[9]
            1535164, -1703391, 152506, -364577, 53808, 310769, -97925, -704908, 822601
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
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
            1136,    // u4MinGain, 1024 base = 1x
            10240,    // u4MaxGain, 16x
            46,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            27,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            27,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            27,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            TRUE,    // bEnableCaptureThres
            TRUE,    // bEnableVideoThres
            TRUE,    // bEnableStrobeThres
            47,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            0,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            4,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            160,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                892,    // i4R
                512,    // i4G
                760    // i4B
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
                -446,    // i4X
                -371    // i4Y
            },
            // A
            {
                -318,    // i4X
                -411    // i4Y
            },
            // TL84
            {
                -202,    // i4X
                -381    // i4Y
            },
            // CWF
            {
                -161,    // i4X
                -444    // i4Y
            },
            // DNP
            {
                -82,    // i4X
                -396    // i4Y
            },
            // D65
            {
                59,    // i4X
                -351    // i4Y
            },
            // DF
            {
                -26,    // i4X
                -380    // i4Y
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
                -476,    // i4X
                -331    // i4Y
            },
            // A
            {
                -352,    // i4X
                -382    // i4Y
            },
            // TL84
            {
                -234,    // i4X
                -362    // i4Y
            },
            // CWF
            {
                -199,    // i4X
                -428    // i4Y
            },
            // DNP
            {
                -116,    // i4X
                -387    // i4Y
            },
            // D65
            {
                29,    // i4X
                -355    // i4Y
            },
            // DF
            {
                -59,    // i4X
                -376    // i4Y
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
                567,    // i4G
                1713    // i4B
            },
            // A 
            {
                581,    // i4R
                512,    // i4G
                1375    // i4B
            },
            // TL84 
            {
                652,    // i4R
                512,    // i4G
                1127    // i4B
            },
            // CWF 
            {
                750,    // i4R
                512,    // i4G
                1161    // i4B
            },
            // DNP 
            {
                784,    // i4R
                512,    // i4G
                979    // i4B
            },
            // D65 
            {
                892,    // i4R
                512,    // i4G
                760    // i4B
            },
            // DF 
            {
                827,    // i4R
                512,    // i4G
                887    // i4B
            }
        },
        // Rotation matrix parameter
        {
            5,    // i4RotationAngle
            255,    // i4Cos
            22    // i4Sin
        },
        // Daylight locus parameter
        {
            -152,    // i4SlopeNumerator
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
            -284,    // i4RightBound
            -934,    // i4LeftBound
            -306,    // i4UpperBound
            -407    // i4LowerBound
            },
            // Warm fluorescent
            {
            -284,    // i4RightBound
            -934,    // i4LeftBound
            -407,    // i4UpperBound
            -527    // i4LowerBound
            },
            // Fluorescent
            {
            -166,    // i4RightBound
            -284,    // i4LeftBound
            -290,    // i4UpperBound
            -395    // i4LowerBound
            },
            // CWF
            {
            -166,    // i4RightBound
            -284,    // i4LeftBound
            -395,    // i4UpperBound
            -478    // i4LowerBound
            },
            // Daylight
            {
            54,    // i4RightBound
            -166,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Shade
            {
            414,    // i4RightBound
            54,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            54,    // i4RightBound
            -166,    // i4LeftBound
            -435,    // i4UpperBound
            -550    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            414,    // i4RightBound
            -934,    // i4LeftBound
            0,    // i4UpperBound
            -550    // i4LowerBound
            },
            // Daylight
            {
            79,    // i4RightBound
            -166,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Cloudy daylight
            {
            179,    // i4RightBound
            4,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Shade
            {
            279,    // i4RightBound
            4,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Twilight
            {
            -166,    // i4RightBound
            -326,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
            },
            // Fluorescent
            {
            79,    // i4RightBound
            -334,    // i4LeftBound
            -305,    // i4UpperBound
            -478    // i4LowerBound
            },
            // Warm fluorescent
            {
            -252,    // i4RightBound
            -452,    // i4LeftBound
            -305,    // i4UpperBound
            -478    // i4LowerBound
            },
            // Incandescent
            {
            -252,    // i4RightBound
            -452,    // i4LeftBound
            -275,    // i4UpperBound
            -435    // i4LowerBound
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
            816,    // i4R
            512,    // i4G
            845    // i4B
            },
            // Cloudy daylight
            {
            964,    // i4R
            512,    // i4G
            694    // i4B
            },
            // Shade
            {
            1026,    // i4R
            512,    // i4G
            645    // i4B
            },
            // Twilight
            {
            636,    // i4R
            512,    // i4G
            1137    // i4B
            },
            // Fluorescent
            {
            777,    // i4R
            512,    // i4G
            1000    // i4B
            },
            // Warm fluorescent
            {
            589,    // i4R
            512,    // i4G
            1390    // i4B
            },
            // Incandescent
            {
            558,    // i4R
            512,    // i4G
            1328    // i4B
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
            0,    // i4SliderValue
            5830    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5280    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1342    // i4OffsetThr
            },
            // Daylight WB gain
            {
            747,    // i4R
            512,    // i4G
            940    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            513,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -505,    // i4RotatedXCoordinate[0]
                -381,    // i4RotatedXCoordinate[1]
                -263,    // i4RotatedXCoordinate[2]
                -145,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace



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

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
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

        default:
            break;
    }
    return 0;
}}; // NSFeature


