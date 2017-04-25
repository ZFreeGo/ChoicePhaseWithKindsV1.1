/****************************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:FrequrncyCaluate.h
*文件标识:
*创建日期： 2016年11月11日
*摘要:
*2016年11月11日: 将测频程序独立成文件，增加模块化.
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
*******************************************************/

#ifndef __ALGORITHMINCLUDE_FREQURNCYCALUATE_H_
#define __ALGORITHMINCLUDE_FREQURNCYCALUATE_H_
#include "F2806x_Examples.h"   // F2806x Examples Include File

//相角参数
typedef struct
{
  float phase; //相位
  float real;  //实部
  float imag;  //虚部
  float im_re; //虚部与实部比值
} AngleElement;




#define PI    3.141592653589793f
#define PI3_2 4.712388980384690f //3/2*pi
#define PID2  1.570796326794897f //  PI/2
#define D2PI  0.159154943091895f  //  1/(2PI)

void CaliAB_Base(float yangben[], AngleElement* pBase);
void GetNewFreq(float yangben[], float f0, float* pFq);
void CalFreq(float* pData);

float  MidMeanFilter(float *pData, Uint8 len);
#endif /* ALGORITHMINCLUDE_FREQURNCYCALUATE_H_ */
