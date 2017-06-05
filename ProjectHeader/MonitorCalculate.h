/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:MonitorCalculate.h
*文件标识:
*创建日期： 2015年1月25日
*摘要:
* 包含基本定义，声明，与类型定义
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#ifndef __MONITORCALCULATE_H_
#define __MONITORCALCULATE_H_
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "FrequrncyCaluate.h"



#define RFFT_STAGES 6
#define RFFT_SIZE (1 << RFFT_STAGES)
#define N_FFT 64






//时间计算与中间量
struct TimeParameteCall
{
	float CalTimeDiff;//计算的时间差值
	float CalTp;   //计算得带的周期
	float CalT0;   //计算得带的时间差
	float CalPhase;//计算得到的相位
};

//外部命令参数合集
struct OrderParamCollect
{
	float HezhaTime; //合闸时间us
	float FenzhaTime; //分闸时间 us
	float SetPhase;   //设定相角 弧度
	float SetPhaseTime; //相位折算为时间 对应Cos
	Uint16 HeFenFlag; //合分闸标识 0-合闸  0xffff 分闸

};

#ifdef	__cplusplus
extern "C" {
#endif

void  InitMonitorCalData(void);

void FFT_Init(void);
void FFT_Cal(float ADsample[]);



void GetOVD(float* pData);
extern void SynchronizTrigger(float* pData);
extern void TestCalculate(void);


extern void CalEffectiveValue(void);
extern float32 RFFToutBuff[RFFT_SIZE]; //from MonitorCalculate.c


#ifdef	__cplusplus
}
#endif

#endif /* MONITORCALCULATE_H_ */
