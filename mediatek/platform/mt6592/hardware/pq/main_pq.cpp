#define LOG_TAG "PQ"

#include <cutils/xlog.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "ddp_drv.h"
#include "ddp_color_index.h"
#include "ddp_tdshp_index.h"

int drvID = -1;
unsigned long lcmindex = 0; 

const DISP_PQ_PARAM pqparam = 
{
u4SHPGain:2,
u4SatGain:0,
u4HueAdj:{9,9,12,12},
u4SatAdj:{0,2,6,6}
};

const DISP_PQ_PARAM pqparam_camera =
{
    u4SHPGain:0,
    u4SatGain:0,
    u4HueAdj:{9,9,9,9},
    u4SatAdj:{0,0,0,0}
};

#ifdef WT_GAMMA_PQ_WITH_MULTI_LCM
int get_gamma_retry_count = 0, get_pq_retry_count = 0;
DISPLAY_GAMMA_T gamma_lcd;
DISP_PQ_PARAM pqparam_lcd;
int gamma_retry_ret = -1;
int pq_retry_ret = -1;
#endif
//COLOR_TUNING_INDEX : 10


//DISPLAY_PQ_T pqindex;
int main(int argc, char** argv) 
{
    int actionID=0, RegBase = 0, RegValue = 0, err = 0;
    char fileName[256];

    XLOGD("PQ init start...");
    if(drvID == -1) //initial
        drvID = open("/dev/mtk_disp", O_RDONLY, 0);

#ifdef WT_GAMMA_PQ_WITH_MULTI_LCM
	while(gamma_retry_ret != 0 && get_gamma_retry_count < 3)
	{
		gamma_retry_ret = ioctl(drvID, DISP_IOCTL_GET_GAMMAPARAM_FROM_LCDDRV, &gamma_lcd);
		get_gamma_retry_count++;
	}

	while(pq_retry_ret != 0 && get_pq_retry_count < 3)
	{
		pq_retry_ret = ioctl(drvID, DISP_IOCTL_GET_PQPARAM_FROM_LCDDRV, &pqparam_lcd);
		get_pq_retry_count++;
	}
#endif
	
    XLOGD("PQ test...");
    ioctl(drvID, DISP_IOCTL_SET_PQINDEX, &pqindex);
#ifdef WT_GAMMA_PQ_WITH_MULTI_LCM
	if(pq_retry_ret == 0)
	{
		ioctl(drvID, DISP_IOCTL_SET_PQPARAM, &pqparam_lcd);
	}
	else
#endif	
	{
    	ioctl(drvID, DISP_IOCTL_SET_PQPARAM, &pqparam);
	}
#ifdef WT_GAMMA_PQ_WITH_MULTI_LCM
if(gamma_retry_ret == 0)
{
    ioctl(drvID, DISP_IOCTL_SET_GAMMALUT, &gamma_lcd);
}
else
#endif
{
    ioctl(drvID, DISP_IOCTL_SET_GAMMALUT, &gammaindex);
}

    ioctl(drvID, DISP_IOCTL_SET_TDSHPINDEX, &tdshpindex);
    ioctl(drvID, DISP_IOCTL_SET_PQ_CAM_PARAM, &pqparam_camera);
#ifdef WT_GAMMA_PQ_WITH_MULTI_LCM
	if(pq_retry_ret == 0)
	{	
    	ioctl(drvID, DISP_IOCTL_SET_PQ_GAL_PARAM, &pqparam_lcd);
	}
	else
#endif
	{
		ioctl(drvID, DISP_IOCTL_SET_PQ_GAL_PARAM, &pqparam);
	}

    XLOGD("PQ init end !");
    return 0;
}
