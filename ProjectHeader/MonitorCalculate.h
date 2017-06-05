/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:MonitorCalculate.h
*�ļ���ʶ:
*�������ڣ� 2015��1��25��
*ժҪ:
* �����������壬�����������Ͷ���
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#ifndef __MONITORCALCULATE_H_
#define __MONITORCALCULATE_H_
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "FrequrncyCaluate.h"



#define RFFT_STAGES 6
#define RFFT_SIZE (1 << RFFT_STAGES)
#define N_FFT 64






//ʱ��������м���
struct TimeParameteCall
{
	float CalTimeDiff;//�����ʱ���ֵ
	float CalTp;   //����ô�������
	float CalT0;   //����ô���ʱ���
	float CalPhase;//����õ�����λ
};

//�ⲿ��������ϼ�
struct OrderParamCollect
{
	float HezhaTime; //��բʱ��us
	float FenzhaTime; //��բʱ�� us
	float SetPhase;   //�趨��� ����
	float SetPhaseTime; //��λ����Ϊʱ�� ��ӦCos
	Uint16 HeFenFlag; //�Ϸ�բ��ʶ 0-��բ  0xffff ��բ

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
