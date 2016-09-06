
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <utils/Log.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int MTK_Tfa98xx_Check_TfaOpen(void)
{
   return tfa9890_check_tfaopen();
}

int MTK_Tfa98xx_Init(void)
{
    int res;
    ALOGD("Tfa98xx: +%s",__func__);
    res = tfa9890_init();
    ALOGD("Tfa98xx: -%s, res= %d",__func__,res);
    return res;
}

int MTK_Tfa98xx_Deinit(void)
{
	  int res = 0;
    ALOGD("Tfa98xx: +%s",__func__);
    if(MTK_Tfa98xx_Check_TfaOpen())
    {
        res = tfa9890_deinit();
    }
    ALOGD("Tfa98xx: -%s, res= %d",__func__, res);
    return res;
}

void MTK_Tfa98xx_SpeakerOn(void)
{
    ALOGD("Tfa98xx: +%s, nxp_init_flag= %d",__func__,MTK_Tfa98xx_Check_TfaOpen());
    if(!MTK_Tfa98xx_Check_TfaOpen())                   //already done in tfa9890_SpeakerOn()
    	MTK_Tfa98xx_Init();

    tfa9890_SpeakerOn();
    ALOGD("Tfa98xx: -%s, nxp_init_flag= %d",__func__,MTK_Tfa98xx_Check_TfaOpen());
}

void MTK_Tfa98xx_SpeakerOff(void)
{
    ALOGD("Tfa98xx: +%s",__func__);
    tfa9890_SpeakerOff();
    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_SetSampleRate(int samplerate)
{
    ALOGD("Tfa98xx: +%s, samplerate=%d",__func__,samplerate);
    tfa9890_setSamplerate(samplerate);
    ALOGD("Tfa98xx: -%s",__func__);
}
void MTK_Tfa98xx_SetBypassDspIncall(int bypass)
{
    ALOGD("Tfa98xx: +%s, bypass= %d",__func__,bypass);
    set_bypass_dsp_incall(bypass);
    ALOGD("Tfa98xx: -%s",__func__);
}

void MTK_Tfa98xx_EchoReferenceConfigure(int config)
{
    ALOGD("Tfa98xx: +%s, nxp_init_flag= %d, config= %d",__func__,MTK_Tfa98xx_Check_TfaOpen(), config);
    if(MTK_Tfa98xx_Check_TfaOpen())
       tfa9890_EchoReferenceConfigure(config);
    ALOGD("Tfa98xx: -%s,",__func__);
}

void MTK_Tfa98xx_Reset()
{
    ALOGD("MTK_Tfa98xx_Reset");
    tfa9890_deinit();
    tfa9890_reset();
    ALOGD("Tfa98xx: -%s,",__func__);
}

EXPORT_SYMBOL(MTK_Tfa98xx_Init);
EXPORT_SYMBOL(MTK_Tfa98xx_Reset);
EXPORT_SYMBOL(MTK_Tfa98xx_Deinit);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOn);
EXPORT_SYMBOL(MTK_Tfa98xx_SpeakerOff);
EXPORT_SYMBOL(MTK_Tfa98xx_SetSampleRate);
EXPORT_SYMBOL(MTK_Tfa98xx_SetBypassDspIncall);
EXPORT_SYMBOL(MTK_Tfa98xx_EchoReferenceConfigure);

