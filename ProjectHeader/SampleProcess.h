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



//���������
#define OVER_SAMPLE_LEN  1
//������ת��ϵ��  ��������ȵĵ���
#define OVER_SAMPLE_RATE 1.0f

void InitSampleProcessData(void);
void  ADC_INT1_ISR(void);



#endif /* SAMPLEPROCESS_H_ */
