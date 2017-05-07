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
#include "RefParameter.h"

/*=============================ȫ�ֱ������� Start=============================*/
//��������ϵ��
float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
float TwoDivideN = 0; //2/N
/*=============================ȫ�ֱ������� End=============================*/

void CaliAB_Base(float yangben[], AngleElement* pBase);
void GetNewFreq(float yangben[], float f0, float* pFq);
void CalFreq(float* pData);

float  MidMeanFilter(float *pData, Uint8 len);


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


/**
 * �����������Ƶ��
 *
 * @param  yangben   �������ڵ�����ֵ
 * @param  f0        ��ǰƵ��
 * @param  *pFq      �������Ƶ��ָ��
 * @brief  ������ʷ����У��Ƶ��
 */
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




/**
 * ����Ƶ��ģ�飬�������ݼ���Ƶ��
 *
 * @param  pData   ָ�������ݵ�ָ�룬��Ҫ��2*N�����ݣ�������ֱ�����ݻ���������
 *
 * @brief   ������ʷ����У��Ƶ��
 */
void CalFreq(float* pData)
{
  if (pData[SAMPLE_LEN] == SAMPLE_COMPLTE) //�Ѿ���ɲ���
    {

      GetNewFreq(pData, g_SystemVoltageParameter.frequencyCollect.FreqReal, &g_SystemVoltageParameter.frequencyCollect.FreqCal); //��ȡ��Ƶ��
      pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //��Ϊ������־

      //�ж�����õ�Ƶ���Ƿ�����ֵ�ڣ�������������ڣ�����Ƶ��ΪĬ��Ƶ�ʣ�������
      if ((g_SystemVoltageParameter.frequencyCollect.FreqCal >=  g_SystemLimit.frequency.min)
    		  && (g_SystemVoltageParameter.frequencyCollect.FreqCal <=  g_SystemLimit.frequency.max))
        {
          g_SystemVoltageParameter.frequencyCollect.FreqReal = g_SystemVoltageParameter.frequencyCollect.FreqCal; //����Ƶ��ΪĬ��Ƶ��  //
        }
      else
        {
          g_SystemVoltageParameter.frequencyCollect.FreqReal = g_SystemVoltageParameter.frequencyCollect.FreqInit; //��ΪĬ��Ƶ��
          //���ʹ�����Ϣ
        }
    }
}


/**
 * ��ֵ�����˲�ģ��
 *
 * @param  pData   ָ����˲�������
 * @param  len     ���ݳ���
 * @return         ƽ���˲����
 * @brief   ��ֵƽ���˲�,ȥ����Сֵ�����ֵ��ȡƽ��ֵ
 */
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

static float freqArray[7] = { 0 };
static Uint8 freqLen = 0;
/**
 * ���¼���Ƶ��
 * <p>
 * ��float��ʽ�洢����4�ֽڽ��н������������
 * @param   null
 *
 * @brif ����ͨѶ��������ʹ��
 */
void UpdateFrequency(void)
{
	float samplePriod = 0;
	//��Ƶģ�鴦��
			//�����Ͻ���20msһ��ѭ��
	//Todo:���������ڱȽ��Ǹ�����
	if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_COMPLTE) //�������
	{
		//����Ƶ��
		CalFreq(SampleDataSavefloat);
		CalEffectiveValue();
		if (freqLen < 7) //�������� ÿ7��ȡƽ��ֵ��������
		{
			freqArray[freqLen++] = g_SystemVoltageParameter.frequencyCollect.FreqReal;

		}
		else
		{
			freqLen = 0;
			g_SystemVoltageParameter.frequencyCollect.FreqMean = g_SystemCalibrationCoefficient.frequencyCoefficient * MidMeanFilter(freqArray, 7); //
		}

		g_SystemVoltageParameter.frequencyCollect.FreqReal = g_SystemVoltageParameter.frequencyCollect.FreqMean;
		g_SystemVoltageParameter.period = 1e6/g_SystemVoltageParameter.frequencyCollect.FreqMean;
		samplePriod = 15625.0f / g_SystemVoltageParameter.frequencyCollect.FreqMean; //����ʵʱ�������� 1e6/64

		SetSamplePriod(samplePriod);
		StartSample(); //������ʼ�������ٴβ�Ƶ
	}

}

