/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:ConfigADC.h
*�ļ���ʶ:
*�������ڣ� 2015��1��31��
*ժҪ:
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#ifndef __CONFIGADC_H_
#define __CONFIGADC_H_

void ConfigADC_Monitor(float priod);//���ü�ز���ADC
void StartSample(void);
void StopSample(void);
void SetSamplePriod(float us);

#endif /* CONFIGADC_H_ */
