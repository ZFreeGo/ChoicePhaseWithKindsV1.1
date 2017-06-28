/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Header.h
*文件标识:
*创建日期： 2015年1月28日  下午2:36:15:创建本文件
*摘要: 此文件包含主要头文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/



#ifndef __HEADER_H_
#define __HEADER_H_

//FFT与ZVD二者选择开关 置一为开启模式
#define WITH_FFT  1
#define WITH_ZVD  0

#if WITH_FFT == 1
//存储长度
#define SAMPLE_LEN  128 //每周波64点 采样两个周波
#define SAMPLE_LEN_PRIOD  64//每周波

#elif WITH_ZVD == 1
#define SAMPLE_LEN  128*10//每周波128*10点 采样两个周波
#define SAMPLE_LEN_PRIOD  128//每周波

#elif
#error "没有适当定义当前采样模式"
#endif
#define SAMPLE_LEN_CAL 64  //相角计算长度

//#include "F2806x_Cla_typedefs.h"// F2806x CLA Type definitions



//单独模式--单独的永磁控制器与独立的永磁同步控制器
//

#define INTEG_MODE  0x5555

#include <math.h>
#include <stdio.h>
#include "string.h"
#include "fpu.h"
#include "C28x_FPU_FastRTS.h"

#include "DeviceIO.h"

#include "ConfigADC.h"

#include "MonitorCalculate.h"
#include "SampleProcess.h"

#include "PwmTimer.h"

#include "Timer.h"

#include "CAN.h"
#include "DeviceNet.h"
#include "stdType.h"
#include "SoftI2C.h"

#ifdef INTEG_MODE
#include "ECAP.h"
#endif


#include "Action.h"

//#include"CLAmath.h"
#define NOP()   __asm(" NOP")
//#define Reset() {__asm(" NOP");}





#define LOCAL_ADDRESS  0xA1 //本机地址
#define MAIN_LOCAL     0xBA //主机地址

#define FREQ_CALI_RATE   1//1.005f //频率校正系数
/**************************************************************************/
#endif /* HEADER_H_ */
