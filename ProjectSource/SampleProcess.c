/***********************************************
*Copyright(c) 2011,FreeGo
*��������Ȩ��
*�ļ�����:SampleProcess.c
*�ļ���ʶ:
*�������ڣ� 2011��1��1��
*ժҪ:
*����11:05:53:�������ļ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/

#include "SampleProcess.h"
#include "Header.h"
#include "DSP28x_Project.h"


/*=============================ȫ�ֱ������� Start=============================*/
Uint16 SampleData[SAMPLE_LEN] = {0}; //�������ݴ洢
Uint16 SampleDataSave[SAMPLE_LEN + 1] = {0}; //�������ݴ洢 ת�� (SAMPLE_LEN + 1)����
//Uint16 �� float �任�����ڿ��Կ������ÿռ䣬����ramʹ��
float SampleDataSavefloat[SAMPLE_LEN + 1] = {0};


Uint16 SampleIndex = 0; //��������
Uint16 i = 0;

Uint16 SampleLen = SAMPLE_LEN;
float SamplePriod = 0;
/*=============================ȫ�ֱ������� End=============================*/

/*=============================���ñ��� Start=============================*/

extern struct FreqCollect FreqMonitor;


volatile Uint8  ZVDFlag = 0;
/*=============================���ñ��� End=============================*/







/********************************************************************
 * ��������InitSampleProcessData()
 * ������NULL
 * ����ֵ��NULL
 * ���ܣ���ʼ����������
 ********************************************************************/
void InitSampleProcessData(void)
{
  Uint16 i = 0;
  for (i = 0; i < SAMPLE_LEN; i++)
    {
      SampleData[i] = 0;
      SampleDataSave[i] = 0;
      SampleDataSavefloat[i] = 0;
    }
  SampleIndex = 0;

  SampleDataSave[SAMPLE_LEN] = SAMPLE_NOT_FULL;
  SampleDataSavefloat[SAMPLE_LEN] = SAMPLE_NOT_FULL;


  SampleLen = SAMPLE_LEN;
#if WITH_FFT == 1
  SamplePriod =  	15625.0f /  FreqMonitor.FreqMean; //������Ƶ��  50hz
#elif  WITH_ZVD == 1
  SamplePriod = 156.25;//7812.5/50 �ָ�ԭ������

#endif

}




/********************************************************************
 * ��������ADC_INT1_ISR()
 * ������NULL
 * ����ֵ��NULL
 * ���ܣ��жϺ�����ADCINT1�ж�
 ********************************************************************/
__interrupt void  ADC_INT1_ISR(void)
{

	AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;           //Clear ADCINT1 flag reinitialize for next SOC
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE


   SampleData[SampleIndex] = AdcResult.ADCRESULT0;
   //SampleData[SampleIndex] = SimSinvalue[SampleIndex];

   SampleIndex++;
   if (SampleIndex == 1)
   {
	   TOGGLE_LED1; //���������־
   }
   if (SampleIndex >= SampleLen)
     {
       StopSample(); //ֹͣ����
       SampleIndex = 0;

       TOGGLE_LED1; //���������־

       //�ж������Ƿ��Ѿ�����   ��û�д��� �򲻽�������ת�棬������������.  �������ݼ���
       if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_NOT_FULL)
         {
           for (i = 0; i < SAMPLE_LEN; i++)
             {
                 SampleDataSave[i] = SampleData[i]; //ת������
                 SampleDataSavefloat[i] = SampleData[i];
             }
           SampleDataSave[SAMPLE_LEN] = SAMPLE_COMPLTE; //���һ����Ϊ�����洢��ɱ�־
           SampleDataSavefloat[SAMPLE_LEN] = SAMPLE_COMPLTE;


           //����ʱ���
           if (ZVDFlag) //�����ʼ����
           {
#if WITH_FFT == 1
        	   ZVDFlag = 0; //������ձ�־����ֹ�ظ�����

        	   GetOVD(SampleDataSavefloat); //7825 7803cyc 80 10us
        	   SampleLen = SAMPLE_LEN; //ԭʼ
        	   //���ָ�״̬
        	  // StartSample();

#elif  WITH_ZVD == 1
        	   ZVDFlag = 0; //������ձ�־����ֹ�ظ�����
        	   GetOVD(SampleDataSavefloat); //7825 7803cyc 80 10us //ͬ��

        	   //��ӻ�ȡ��������
        	   SampleLen = SAMPLE_LEN; //ԭʼ
        	   SamplePriod = 156.25; //�ָ�ԭ�в�������
#elif
#error "Ϊ�ʵ����嵱ǰ����ģʽ"
#endif
           }
         }
       else
         {
            //����δת��״ָ̬ʾ
         }

     }


   return;
}
