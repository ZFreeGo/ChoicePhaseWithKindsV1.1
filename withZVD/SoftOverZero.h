/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:SoftOverZero.h
*�ļ���ʶ:
*�������ڣ� 2015��2��1��
*ժҪ:
*����5:28:51:�������ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#ifndef SOFTOVERZERO_H_
#define SOFTOVERZERO_H_
#include "DSP28x_Project.h"


void FIR_Sample();
void FIR_Self(float* b, Uint16 blen, float *x,  Uint16 len, float* out);
Uint16 SearchZero(float* array,Uint16 start,Uint16 endIndex,
				float offset, float* index);
#endif /* SOFTOVERZERO_H_ */
