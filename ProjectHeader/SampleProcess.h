/***********************************************
*Copyright(c) 2011,FreeGo
*��������Ȩ��
*�ļ�����:SampleProcess.h
*�ļ���ʶ:
*�������ڣ� 2011��1��1��
*ժҪ:
*����11:05:41:�������ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/



#ifndef SAMPLEPROCESS_H_
#define SAMPLEPROCESS_H_

#define SAMPLE_COMPLTE  0x5555
#define SAMPLE_NOT_FULL 0xAAAA


#include "Header.h"
#include "RefParameter.h"


//���������
#define OVER_SAMPLE_LEN  1
//������ת��ϵ��  ��������ȵĵ���
#define OVER_SAMPLE_RATE 1.0f

#define SYN_MODE 0xAAAA
#define FREQ_MODE 0x5555



void InitSampleProcessData(void);
void  ADC_INT1_ISR(void);

//ȫ�ֱ���
extern uint16_t SampleDataSave[SAMPLE_LEN + 1]; //�������ݴ洢 ת�� (SAMPLE_LEN + 1)����  from SampProcess.c
extern float SampleDataSavefloat[SAMPLE_LEN + 1]; //Uint16 �� float �任�����ڿ��Կ������ÿռ䣬����ramʹ��  from SampProcess.c
extern void ChangeSampleMode(uint16_t mode);
#endif /* SAMPLEPROCESS_H_ */
