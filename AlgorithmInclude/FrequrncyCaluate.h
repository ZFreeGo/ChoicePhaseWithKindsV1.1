/****************************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:FrequrncyCaluate.h
*�ļ���ʶ:
*�������ڣ� 2016��11��11��
*ժҪ:
*2016��11��11��: ����Ƶ����������ļ�������ģ�黯.
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
*******************************************************/

#ifndef __ALGORITHMINCLUDE_FREQURNCYCALUATE_H_
#define __ALGORITHMINCLUDE_FREQURNCYCALUATE_H_
#include "F2806x_Examples.h"   // F2806x Examples Include File

//��ǲ���
typedef struct
{
  float phase; //��λ
  float real;  //ʵ��
  float imag;  //�鲿
  float im_re; //�鲿��ʵ����ֵ
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
