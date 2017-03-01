/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:Header.h
*�ļ���ʶ:
*�������ڣ� 2015��1��28��  ����2:36:15:�������ļ�
*ժҪ: ���ļ�������Ҫͷ�ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/



#ifndef __HEADER_H_
#define __HEADER_H_

//FFT��ZVD����ѡ�񿪹� ��һΪ����ģʽ
#define WITH_FFT  1
#define WITH_ZVD  0

#if WITH_FFT == 1
//�洢����
#define SAMPLE_LEN  128 //ÿ�ܲ�64�� ���������ܲ�
#define SAMPLE_LEN_PRIOD  64//ÿ�ܲ�

#elif WITH_ZVD == 1
#define SAMPLE_LEN  128*10//ÿ�ܲ�128*10�� ���������ܲ�
#define SAMPLE_LEN_PRIOD  128//ÿ�ܲ�

#elif
#error "û���ʵ����嵱ǰ����ģʽ"
#endif
#define SAMPLE_LEN_CAL 64  //��Ǽ��㳤��

//#include "F2806x_Cla_typedefs.h"// F2806x CLA Type definitions

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
//#include"CLAmath.h"
#define NOP()   __asm(" NOP")
//#define Reset() {__asm(" NOP");}





#define FALSE 0UL
#define TRUE 0xff

#define LOCAL_ADDRESS  0xA1 //������ַ
#define MAIN_LOCAL     0xBA //������ַ

#define FREQ_CALI_RATE   1//1.005f //Ƶ��У��ϵ��
/**************************************************************************/
#endif /* HEADER_H_ */