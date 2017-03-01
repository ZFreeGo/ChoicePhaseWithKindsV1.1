/****************************************************
*Copyright(c) 2016,FreeGo
*��������Ȩ��
*�ļ�����:FrequrncyCaluate.c
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
#include "FrequrncyCaluate.h"
#include "Header.h"


/*=============================ȫ�ֱ������� Start=============================*/
//��������ϵ��
float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
float TwoDivideN = 0; //2/N
/*=============================ȫ�ֱ������� End=============================*/
extern float32 RFFToutBuff[RFFT_SIZE]; //from MonitorCalculate.c
extern struct  FreqCollect FreqMonitor; //from MonitorCalculate.c
/*=============================���ñ��� extern Start=============================*/

/**************************************************
 *��������CaliAB_Base()
 *���ܣ� �������ֱ��˥������ʱ�Ĳ���ֵ��������ʵʱ��λֵ
 *�βΣ�float yangben[] ����ֵ��������ֱ��ADC�����˲����ֵ��AngleElement* pBase ���ݵ�������λ
 *����ֵ��void
 *���ú����� atan()
 *�����ⲿ������TwoDivideN, Cos1step
****************************************************/
void CaliAB_Base(float yangben[],AngleElement* pBase)
{
	float DA;  //Ϊ�˸��ٵ���
	float DB;
	float calRel;
	float calImag;
	float A = 0, B = 0;
    float fzSum = 0;
    float fmSum = 0;
    float sum = 0;

    Uint16 i = 0;
    Uint16 len = RFFT_SIZE>>1;
    for (i = 0; i < len; i++)
      {
        fzSum += yangben[2*i + 1]; //1,3,  ������������ͣ���Ӧ����ż����
        fmSum += yangben[2*i];//0,2,4

      }
    B = fzSum / fmSum;  //��B��
    //���������
    sum = fzSum + fmSum;
    /*
    for (i = 0; i < RFFT_SIZE; i++)
     {
        sum += yangben[i];
      }
      */
    A = sum * (1-B);  //��ϵ��A

    //����У��ֵ  ��ȡ������λ
    float  fm = 1,  fmrate = 1;
    //fm = 1 - 2 * B * Cos1step + B * B;
    fm = 1 - (2 * Cos1step - B) * B;
    fmrate = 1.0 / fm;
    DA = TwoDivideN * A * (1 - B * Cos1step) * fmrate; //�ɿ�����ȥTwoDivideN
    DB = TwoDivideN * A * B * Sin1step * fmrate;

    calRel = RFFToutBuff[1] * TwoDivideN - DA; //����ʵ������
    calImag = RFFToutBuff[RFFT_SIZE - 1]*TwoDivideN + DB; //�����鲿����

    pBase->real = calRel;
    pBase->imag = calImag;
    pBase->im_re = calImag / calRel;
    pBase->phase = atan(pBase->im_re);

   /* pBase->real = RFFToutBuff[1];
    pBase->imag = RFFToutBuff[RFFT_SIZE - 1];
    pBase->im_re =  RFFToutBuff[RFFT_SIZE - 1]/ RFFToutBuff[1] ;
    pBase->phase = atan(pBase->im_re);
    */
    if (calRel < 0)
      {
         if(calImag >= 0)
           {
             pBase->phase += PI;
           }
         else
           {
             pBase->phase -= PI;
           }
      }
}

/**************************************************
 *��������GetNewFreq()
 *���ܣ������������Ƶ��
 *�βΣ�float yangben[] �������ڵ�����ֵ, float f0 ��ǰƵ��, float* pFq �������Ƶ��
 *����ֵ��void
 *���ú�����
 *�����ⲿ������
****************************************************/
void GetNewFreq(float yangben[], float f0, float* pFq)
{
	float df1 = 0, r = 0, dA = 0;
  AngleElement baseA, baseB; //�����м���Ϣ

  FFT_Cal(yangben);
 // CaliAB_Base(yangben, &baseA);
  baseA.real = RFFToutBuff[1];
  baseA.imag = RFFToutBuff[RFFT_SIZE - 1];
  baseA.im_re =  RFFToutBuff[RFFT_SIZE - 1]/ RFFToutBuff[1] ;
  baseA.phase = atan(baseA.im_re);
  FFT_Cal(yangben + RFFT_SIZE);
  //CaliAB_Base(yangben + RFFT_SIZE, &baseB);
  baseB.real = RFFToutBuff[1];
  baseB.imag = RFFToutBuff[RFFT_SIZE - 1];
  baseB.im_re =  RFFToutBuff[RFFT_SIZE - 1]/ RFFToutBuff[1] ;
  baseB.phase = atan(baseB.im_re);

  df1 = f0*baseA.phase*D2PI;
  r = f0/(f0 + df1);

  dA = atan(baseB.im_re * r) - atan(baseA.im_re * r);
  //dA����   ʵ�ʹ����в�����ƫ�����PI
  if (dA < 0.5 * PI)
    {
      dA = dA + PI;
    }
  if (dA > 0.5 * PI)
    {
      dA = dA - PI;
    }
  *pFq = f0* (1 + dA*D2PI);
}



/********************************************************************
 * ��������CalFreq()
 * ������Uint16* pData -- ��Ҫ��2*N�����ݣ�������ֱ�����ݻ���������
 * ����ֵ��NULL
 * ���ܣ�����Ƶ��ģ�飬�������ݼ���Ƶ��
 *���ú�����
 *�����ⲿ������
 ********************************************************************/
void CalFreq(float* pData)
{
  if (pData[SAMPLE_LEN] == SAMPLE_COMPLTE) //�Ѿ���ɲ���
    {

      GetNewFreq(pData, FreqMonitor.FreqReal, &FreqMonitor.FreqCal); //��ȡ��Ƶ��
      pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //��Ϊ������־
      //�ж�����õ�Ƶ���Ƿ�����ֵ�ڣ�������������ڣ�����Ƶ��ΪĬ��Ƶ�ʣ�������
      if (FreqMonitor.FreqCal >= FREQ_MIN && FreqMonitor.FreqCal <= FREQ_MAX)
        {
          FreqMonitor.FreqReal = FreqMonitor.FreqCal; //����Ƶ��ΪĬ��Ƶ��  //
        }
      else
        {
          FreqMonitor.FreqReal = FreqMonitor.FreqInit; //��ΪĬ��Ƶ��
          //���ʹ�����Ϣ
        }
    }
}

/********************************************************************
 * ��������MidMeanFilter()
 * ������float *pData -- ָ����������, Uint8 len -- ���ݳ���
 * ����ֵ��float ��ֵ�˲����
 * ���ܣ���ֵƽ���˲�,ȥ����Сֵ�����ֵ��ȡƽ��ֵ
 * ���ú�����null
 * �����ⲿ������null
 ********************************************************************/
float  MidMeanFilter(float *pData, Uint8 len)
{
	float min = 0, max = 0, sum = 0;
	float res = 0; //���ս��
	Uint8 i = 0;
	min = pData[0];
	max=  pData[0];

	for (i = 0; i < len; i++)
	{
		sum += pData[i]; //���
		if(pData[i] < min)
		{
			min = pData[i];
		}
		if (pData[i] > max)
		{
			max = pData[i];
		}

	}
	sum = sum - min - max;
	res = sum / (len - 2);
	return res;
}


