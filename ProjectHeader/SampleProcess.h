/***********************************************
*Copyright(c) 2011,FreeGo
*保留所有权利
*文件名称:SampleProcess.h
*文件标识:
*创建日期： 2011年1月1日
*摘要:
*上午11:05:41:创建本文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/



#ifndef SAMPLEPROCESS_H_
#define SAMPLEPROCESS_H_

#define SAMPLE_COMPLTE  0x5555
#define SAMPLE_NOT_FULL 0xAAAA


#include "Header.h"
#include "RefParameter.h"


//过采样深度
#define OVER_SAMPLE_LEN  1
//过采样转换系数  过采样深度的倒数
#define OVER_SAMPLE_RATE 1.0f

#define SYN_MODE 0xAAAA
#define FREQ_MODE 0x5555



void InitSampleProcessData(void);
void  ADC_INT1_ISR(void);

//全局变量
extern uint16_t SampleDataSave[SAMPLE_LEN + 1]; //采样数据存储 转存 (SAMPLE_LEN + 1)长度  from SampProcess.c
extern float SampleDataSavefloat[SAMPLE_LEN + 1]; //Uint16 到 float 变换，后期可以考虑重用空间，减少ram使用  from SampProcess.c
extern void ChangeSampleMode(uint16_t mode);
#endif /* SAMPLEPROCESS_H_ */
