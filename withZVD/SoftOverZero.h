/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:SoftOverZero.h
*文件标识:
*创建日期： 2015年2月1日
*摘要:
*下午5:28:51:创建本文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#ifndef SOFTOVERZERO_H_
#define SOFTOVERZERO_H_
#include "DSP28x_Project.h"


void FIR_Sample();
void FIR_Self(float* b, Uint16 blen, float *x,  Uint16 len, float* out);
Uint16 SearchZero(float* array,Uint16 start,Uint16 endIndex,
				float offset, float* index);
#endif /* SOFTOVERZERO_H_ */
