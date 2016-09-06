#ifndef _VMC8178AF_H
#define _VMC8178AF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define VMC8178AF_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
//Support SR?
bool          bIsSupportSR;
} stVMC8178AF_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define VMC8178AFIOC_G_MOTORINFO _IOR(VMC8178AF_MAGIC,0,stVMC8178AF_MotorInfo)

#define VMC8178AFIOC_T_MOVETO _IOW(VMC8178AF_MAGIC,1,unsigned long)

#define VMC8178AFIOC_T_SETINFPOS _IOW(VMC8178AF_MAGIC,2,unsigned long)

#define VMC8178AFIOC_T_SETMACROPOS _IOW(VMC8178AF_MAGIC,3,unsigned long)

#else
#endif
