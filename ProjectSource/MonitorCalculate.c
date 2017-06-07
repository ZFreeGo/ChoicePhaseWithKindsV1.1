/****************************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:MonitorCalculate.c
*�ļ���ʶ:
*�������ڣ� 2015��1��25��
*ժҪ:
*2105/3/24: ���ۺϱ���Ϊ�ṹ��
*2015/1/31: ������ADC���ݵĴ�������
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
*******************************************************/
#include "DSP28x_Project.h"
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "Header.h"
#include "RefParameter.h"
#include "Action.h"

#include <math.h>
#include <stdlib.h>
/*=============================ȫ�ֱ������� Start=============================*/
//#define RFFT_STAGES 6
//#define RFFT_SIZE (1 << RFFT_STAGES)
/*FFTin1Buff section to 2*FFT_SIZE in the linker file*/

//FFT�任�������ڴ����
#pragma DATA_SECTION(RFFTin1Buff,"RFFTdata1");
float32 RFFTin1Buff[RFFT_SIZE];
#pragma DATA_SECTION(RFFToutBuff,"RFFTdata2");
float32 RFFToutBuff[RFFT_SIZE];
#pragma DATA_SECTION(RFFTmagBuff,"RFFTdata3");
float32 RFFTmagBuff[RFFT_SIZE/2+1];
#pragma DATA_SECTION(RFFTF32Coef,"RFFTdata4");
float32 RFFTF32Coef[RFFT_SIZE];

//#pragma DATA_SECTION(RFFphaseBuff,"RFFTdata4");
float32 RFFphaseBuff[RFFT_SIZE/2+1];
RFFT_F32_STRUCT rfft; //FFT �任���ݽṹ��

//��������ϵ��
//float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
//float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
//float TwoDivideN = 0; //2/N


//struct  FreqCollect  g_SystemVoltageParameter.frequencyCollect; //���Ƶ��


volatile struct  TimeParameteCall CalTimeMonitor; //����ʱ�����
volatile struct  TimeParameteCall* pCalTimeMonitor;



volatile  struct  OrderParamCollect SetParam;// �趨����

volatile Uint8 FirstTrig = 0;// �״δ�����־ 0--����Ҫ���� ��0--��Ҫ����
/*=============================ȫ�ֱ������� End=============================*/

/*=============================�ֲ����� Start=============================*/
static uint8_t GetMaxActionTime(ActionRad* pActionRad);
static int8_t GetTimeDiff(float sumTime1, float sumTime2, float period, float diff);
static uint8_t PulseOutTrigger(ActionRad* pActionRad);
static uint8_t CheckActionTime(ActionRad* pActionRad);
static uint8_t CalculateDelayTime(ActionRad* pActionRad, float phase);
/*=============================�ֲ����� End=============================*/



#define N_MAX  20 //���������
#define ERROR_VALUE 10//��������� 10us

/**************************************************
 *��������InitMonitorCalData ()
 *���ܣ� ��ʼ��FFTģ�飬��ʼ��������RFFT
 *�βΣ�void
 *����ֵ��void
 *���ú�����FFT_Init()
 *�����ⲿ������
       RFFphaseBuff[i], RFFTmagBuff[i],RFFTF32Coef[i],
       RFFToutBuff[i], RFFTin1Buff[i]
       Cos1step, Sin1step
       g_SystemVoltageParameter.frequencyCollect, pg_SystemVoltageParameter.frequencyCollect
       CalTimeMonitor, pg_SystemVoltageParameter.frequencyCollect
       SetParam
****************************************************/
void InitMonitorCalData(void)
{
   Uint16 i =0;
  for(i=0;i<RFFT_SIZE;i++)
    {
        RFFphaseBuff[i] = 0;
        RFFTmagBuff[i] = 0;
        RFFTF32Coef[i] = 0;
        RFFToutBuff[i] = 0;
        RFFTin1Buff[i] = 0;
    }

  //Cos1step =cos(2.0*PI*1.0/ (float)RFFT_SIZE);//step = 1
  //Sin1step =sin(2.0*PI*1.0/ (float)RFFT_SIZE);//step = 1
  //TwoDivideN = 1;//TwoDivideN = 2.0 / (float)RFFT_SIZE; ��ȥ
  FFT_Init();


  //pg_SystemVoltageParameter.frequencyCollect = &g_SystemVoltageParameter.frequencyCollect;

  CalTimeMonitor.CalTimeDiff = 0;
  CalTimeMonitor.CalTp = 0;
  CalTimeMonitor.CalT0 = 0;
  CalTimeMonitor.CalPhase = 0;
  pCalTimeMonitor = &CalTimeMonitor;
  FirstTrig = 0;

  SetParam.HezhaTime = 50000;
  SetParam.FenzhaTime = 20000;
  SetParam.SetPhase = 0; //�˴���COS��׼ ����
  SetParam.SetPhaseTime = 20000 * 0.75f;
  SetParam.HeFenFlag = 0;//��բ

  RefParameterInit();
}
/**************************************************
 *��������FFT_Init()
 *���ܣ� ��ʼ��FFT ��������
 *�βΣ�
 *����ֵ��void
 *���ú����� null
 *�����ⲿ������ rfft
****************************************************/
void FFT_Init(void)
{
  NOP();
  rfft.FFTSize = RFFT_SIZE;
  rfft.FFTStages = RFFT_STAGES;
  rfft.InBuf = &RFFTin1Buff[0]; /*Input buffer*/
  rfft.OutBuf = &RFFToutBuff[0]; /*Output buffer*/
  rfft.CosSinBuf = &RFFTF32Coef[0]; /*Twiddle factor buffer*/
  rfft.MagBuf = &RFFTmagBuff[0]; /*Magnitude buffer*/
  rfft.PhaseBuf  = &RFFphaseBuff[0];

}

/**************************************************
 *��������FFT_Cal ()
 *���ܣ� FFT����
 *�βΣ�float ADsample[] -- ADC����ֵ
 *����ֵ��void
 *���ú�����RFFT_f32_sincostable(), RFFT_f32()
 *�����ⲿ������ RFFTin1Buff[], rfft
****************************************************/
void FFT_Cal(float ADsample[])
{

	Uint16 j=0;
	//�����ݸ������
	for(j=0;j<RFFT_SIZE;j++) 
	{
	    RFFTin1Buff[j] = (float32)ADsample[j];
	}
	//rfft.FFTSize = RFFT_SIZE;
	//rfft.FFTStages = RFFT_STAGES;
	//rfft.InBuf = &RFFTin1Buff[0]; /*Input buffer*/
	//rfft.OutBuf = &RFFToutBuff[0]; /*Output buffer*/
	//rfft.CosSinBuf = &RFFTF32Coef[0]; /*Twiddle factor buffer*/
	//rfft.MagBuf = &RFFTmagBuff[0]; /*Magnitude buffer*/
	//rfft.PhaseBuf  = &RFFphaseBuff[0];
	RFFT_f32_sincostable(&rfft); /*5130 Calculate twiddle factor */
	NOP();
	RFFT_f32(&rfft); /*1278 Calculate output*/
	NOP();
	//RFFT_f32_mag(&rfft); /*695	Calculate magnitude	*/
	NOP();
	//RFFT_f32_phase(&rfft); /*1229	Calculate phase	*/
	NOP();

}

/**
 * ʹ��FFTֵ������Чֵ
 */
void CalEffectiveValue(void)
{
	uint16_t j = 0;
	float sum = 0;
	float basic = RFFToutBuff[0]/RFFT_SIZE;
	 //����ʵ�����鲿ƽ������Ϊ��Чֵ ����
	 // g_SystemVoltageParameter.voltageA = sqrt( RFFToutBuff[1]*RFFToutBuff[1] + RFFTmagBuff[1]*RFFTmagBuff[1]) * g_SystemCalibrationCoefficient.voltageCoefficient1;
	  for(j=0;j<RFFT_SIZE;j++)
	  {
		  sum += (RFFTin1Buff[j] -  basic) * (RFFTin1Buff[j] -  basic); //
	  }
	  g_SystemVoltageParameter.voltageA = sqrt(sum/RFFT_SIZE) *  g_SystemCalibrationCoefficient.voltageCoefficient1;


}

/**
 * ͬ�������������ò������ݼ���ͬ����������ʱ�̣�������������,��A��Ϊ��
 *
 * @param  *pData   ָ��������ݵ�ָ��
 * @brief  ���㴥��ʱ�̣�������������
 */
void SynchronizTrigger(float* pData)
{
	float phase = 0.0,a = 0, b = 0;
	uint16_t xiang = 0, i = 0;
	uint16_t  test_result = 0;
	float   calTimeA = 0, calTimeB = 0,diffA = 0;
	FFT_Cal(pData);   //����Ҷ�仯������� ÿ������
	a = RFFToutBuff[1]; //ʵ��
	b = RFFToutBuff[RFFT_SIZE - 1]; //�鲿
	phase = atan( b/a ); //��ȡ��λ(-pi/2 pi/2)


	//��λ�ж�
		if (phase >= 0 ) //��Ϊ�ڵ�1,3����   �����������ж�����,�Ƿ���Ҫ���⴦��
		{
			if (a >= 0)
			{
				xiang = 1; //�ڵ�һ����

			}
			else
			{
				xiang = 3; //�ڵ�������
			}
		}
		else
		{
			if (a >= 0)
			{
				xiang = 4; //�ڵ�������
			}
			else
			{
				xiang = 2; //�ڵڶ�����
			}
		}
		phase += PID2;//cosת��Ϊsin
		if (phase > PI2)//����2PI
		{
			phase = phase - PI2;
		}

		//���ޱ任
		switch (xiang)
		{
			case 1:
			{

				break;
			}
			case 2:
			{
				phase += PI;
				break;
			}
			case 3:
			{
				phase += PI;
				break;
			}
			case 4:
			{
				phase += 2*PI;
			}
		}

		test_result = CalculateDelayTime(g_PhaseActionRad, phase);
		if (test_result != 0)
		{
			SynActionAck(0xA1);
			return;
		}

		test_result = CalculateDelayTime(g_PhaseActionRad + 1, phase);
		if (test_result != 0)
		{
			SynActionAck(0xA1);
			return;
		}
		test_result = CalculateDelayTime(g_PhaseActionRad + 2, phase);
		if (test_result != 0)
		{
			SynActionAck(0xA1);
			return;
		}



		test_result = CheckActionTime(g_PhaseActionRad);
		//����󱨴�
		if (test_result != 0)
		{
			SynActionAck(0xA1);
			return;
		}

		//����ʱ���
		for(i = 1; i < g_PhaseActionRad->count; i++)
		{
			calTimeA = g_ProcessDelayTime[0].calDelayCheck + g_ProcessDelayTime[0].sumDelay;
			calTimeB = g_ProcessDelayTime[1].calDelayCheck + g_ProcessDelayTime[1].sumDelay;
			diffA = fabsf(calTimeB - calTimeA - (g_PhaseActionRad[1].realTime  - g_PhaseActionRad[0].realTime));
			if (diffA > 3)
			{
				SynActionAck(0xA2);
				return;//У�����
			}
		}
		test_result = PulseOutTrigger(g_PhaseActionRad);
		if (test_result!=0)
		{
			SynActionAck(0xA3);
		}
		SendMultiFrame(&g_NetSendFrame);
		SynActionAck(0);
}

/**
 * ���㶯����ʱʱ��
 *
 * @param  pActionRad   ��������
 * @param  phase	    ��ʼ���
 *
 * @return 0-����   ��0-����
 * @brief  ���㴥��ʱ�̣�������������
 */
static uint8_t CalculateDelayTime(ActionRad* pActionRad, float phase)
{

	uint16_t count = 0;
	uint8_t selectPhase = 0;
	float time = 0, difftime = 0;

	//δʹ������
	if (pActionRad->enable == 0)
	{
		return 0;
	}
	//phase С�ڵ���3
	if ( pActionRad->phase >= 3)
	{
		return 0xF1;
	}
	//TODO:�ݶ��ڲ���ʱΪ88us
	g_ProcessDelayTime[selectPhase].innerDelay = 88;
	selectPhase = pActionRad->phase;

	//���㿪ʼʱ��
	pActionRad->startTime = g_SystemVoltageParameter.period * phase * D2PI;
	//�˴���ˣ�Ϊ�˱�֤ʹ�����µ�����
	pActionRad->realTime = g_SystemVoltageParameter.period
			* pActionRad->realRatio;
	pActionRad->realDiffTime = g_SystemVoltageParameter.period
			* pActionRad->realDiffRatio;
	//������ʱ֮��
	g_ProcessDelayTime[selectPhase].sumDelay =
			(float)g_ProcessDelayTime[selectPhase].sampleDelay + (float)g_ProcessDelayTime[selectPhase].pulseDelay
					+ (float)g_ProcessDelayTime[selectPhase].transmitDelay
					+ (float)g_ProcessDelayTime[selectPhase].actionDelay
					+ (float)g_ProcessDelayTime[selectPhase].innerDelay;
	//�ܵ�ʱ���
	time = (float)g_ProcessDelayTime[selectPhase].sumDelay + (float)pActionRad->startTime
			- (float)pActionRad->realTime;

	count = 1;
	do
	{
		difftime = count *  g_SystemVoltageParameter.period - time;
		//�ж�ʱ����Ƿ����0��������������
		if(difftime > 0)
		{
			break;
		}
		count++;

	}while(count < 100);
	//����쳣�ж�
	if (count == 100)
	{
		return 0xF1;
	}



	if (difftime > 0)
	{
		//��ʱ��֮�ʹ��ڵ���0���������������������һ�����ڵ���ʱ
		time = difftime + g_ProcessDelayTime[selectPhase].compensationTime;
		if ( time >= 0 )
		{
			g_ProcessDelayTime[selectPhase].calDelay =  (uint16_t)time;
		}
		else
		{
			time = g_SystemVoltageParameter.period + time;
			if (time >= 0)
			{
				g_ProcessDelayTime[selectPhase].calDelay =  (uint16_t)time;
			}
			else
			{
				g_ProcessDelayTime[selectPhase].calDelay = difftime;//���޺󲻽��в�����
			}


		}
		//Ĭ�ϸ�ֵ
		g_ProcessDelayTime[selectPhase].calDelayCheck = g_ProcessDelayTime[selectPhase].calDelay;
		return 0;
	}
	else
	{
		//�쳣����
		return 0xF2;
	}
}

/**
 * У׼���㶯��ʱ��
 *
 * @param  pActionRad   ָ����ʱ������
 *
 * @return 0-�������� ��0--�д������
 * @brief  ���㴥��ʱ�̣�������������
 */


static uint8_t CheckActionTime(ActionRad* pActionRad)
{
	uint8_t maxIndex = 0;
	int8_t diff_result = 0;
	uint8_t  i = 0,select1 = 0, select2 = 0;
	float t1 = 0, t2 = 0;
	//��ȡ�������
	maxIndex = GetMaxActionTime(pActionRad);
			//����Ȼ���count=1����ʾ����ҪУ׼
	if ((maxIndex == pActionRad->count) || ( pActionRad->count == 1) )
	{
		return 0;
	}

	//С��maxIndex��������У׼
	for ( i = maxIndex; i > 0; i--)
	{
		select1 = pActionRad[i-1].phase;
		t1 = g_ProcessDelayTime[select1].sumDelay + g_ProcessDelayTime[select1].calDelayCheck ;//���У׼�������
		select2 = pActionRad[i].phase;
		t2 = g_ProcessDelayTime[select2].sumDelay + g_ProcessDelayTime[select2].calDelayCheck;
		diff_result = GetTimeDiff(t1, t2, g_SystemVoltageParameter.period, pActionRad[i].realDiffTime);
		NOP();
		if (diff_result != (int8_t)N_MAX)
		{
			if (diff_result == 0)
			{
				continue;
			}
			else if (diff_result > 0)
			{
				g_ProcessDelayTime[select1].calDelayCheck = g_ProcessDelayTime[select1].calDelay + diff_result *g_SystemVoltageParameter.period;
			}
			else
			{
				g_ProcessDelayTime[select2].calDelayCheck = g_ProcessDelayTime[select2].calDelay - diff_result *g_SystemVoltageParameter.period;
			}
		}
		else
		{
			return 0xFF;
		}
	}
	//����maxIndex��������У׼
	for ( i = maxIndex; i < pActionRad->count - 1; i++)
	{
		select1 = pActionRad[i].phase;
		t1 = g_ProcessDelayTime[select1].sumDelay + g_ProcessDelayTime[select1].calDelayCheck ;
		select2 = pActionRad[i + 1].phase;
		t2 = g_ProcessDelayTime[select2].sumDelay + g_ProcessDelayTime[select2].calDelayCheck;
		diff_result = GetTimeDiff(t1, t2, g_SystemVoltageParameter.period, pActionRad[i + 1].realDiffTime);
		if (diff_result != N_MAX)
		{
			if (diff_result == 0)
			{
				g_ProcessDelayTime[select1].calDelayCheck = g_ProcessDelayTime[select1].calDelay;//ֱ�Ӹ�ֵ
				continue;
			}
			else if (diff_result > 0)
			{
				g_ProcessDelayTime[select1].calDelayCheck = g_ProcessDelayTime[select1].calDelay + diff_result *g_SystemVoltageParameter.period;
			}
			else
			{
				g_ProcessDelayTime[select2].calDelayCheck = g_ProcessDelayTime[select2].calDelay - diff_result *g_SystemVoltageParameter.period;
			}
		}
		else
		{
			return 0xFF;
		}
	}
	return 0;
}

/**
 * ����ʱ���ֵ
 *
 * @param sumTime1 �ۼӼ�ʱ1
 * @param sumTime2 �ۼӼ�ʱ2
 * @param period   ����
 * @param diff     ��ֵ
 *
 * @return  n = N_MAX --�������
		   n = 0����ǰֵ����Ҫ�䶯
		   n > 0----SumTime1��Ҫ����n*period
		   n <0 ----SumTime2��Ҫ����n*period
		   
   @brief �����ֵ
 */

static int8_t GetTimeDiff(float sumTime1, float sumTime2, float period, float diff)
{
	int8_t  n = 0;//ѭ������
	float time = 0;
	n = 0;
    if((sumTime1 - sumTime2 + diff) > ERROR_VALUE)
    {
    	//T1 ����T2��T2��Ҫ����������
    	do
    	{
    		n++;
			time = sumTime2 + n * period - sumTime1 - diff;
    		if(time >= ( 0- ERROR_VALUE * n))
    		{
    			break;
    		}
    		if (n < N_MAX)
    		{
    			 continue;
    		}
    		else
    	    {
    			 return (int8_t)N_MAX;
    	    }
    	}while(n < N_MAX);
		
		if(time <=  ERROR_VALUE * n)
		{
			 return -n;
		}
    }
    else if ((sumTime1 - sumTime2 + diff) < -ERROR_VALUE)
    {
		//T2 ����T1��T1��Ҫ����������
		do
    	{
    		n++;
			time = sumTime1 + n * period - sumTime2 + diff;
    		if(time >= ( 0- ERROR_VALUE * n))
    		{
    			break;

    		}
    		if (n < N_MAX)
    		{
    			 continue;
    		}
    		else
    	    {
    			 return (int8_t)N_MAX;
    	    }
    	}
    	while(n < N_MAX);
		
		if(time <=  ERROR_VALUE * n)
		{
			 return n;
		}

    }
    else
    {
    	return 0;
    }
    return N_MAX;
}

/**
 * ����ʱ���ֵ
 *
 * @param pActionRad ��������
 *
 * @return ������������Ϊcount����������ʾȫ�����
 * @brief �����ֵ
 */
static uint8_t GetMaxActionTime(ActionRad* pActionRad)
{
	uint8_t i = 0;
	uint8_t count = pActionRad->count;
	uint8_t maxIndex = 0, maxFlag = 0;
	uint8_t selectPhase = 0;
	float sum = 0, maxSum = 0;


	selectPhase = pActionRad[0].phase;
	//ʱ��֮��=����ʱ + ������ʱ + ��������һ������ǰʱ��
	maxSum = g_ProcessDelayTime[selectPhase].sumDelay + g_ProcessDelayTime[selectPhase].calDelay + pActionRad[count - 1].realTime - pActionRad[0].realTime;

	for (i = 1; i < count; i++)
	{
		selectPhase = pActionRad[i].phase;
		//������ʱ��
		sum = g_ProcessDelayTime[selectPhase].sumDelay + g_ProcessDelayTime[selectPhase].calDelay + pActionRad[count - 1].realTime - pActionRad[i].realTime;
		//�Ƿ����MaxSum
		if ((sum - maxSum ) > ERROR_VALUE)
		{
			maxFlag = 0xFF;
			maxIndex = i;
			maxSum = sum;
		}
		else if (( maxSum - sum) > ERROR_VALUE)
		{
			maxFlag = 0xFF;
			maxSum = sum;
		}
	}
	if (maxFlag == 0xFF)
	{
		return maxIndex;
	}
	else
	{
		return count;
	}

}
/**
 * ͬ��������崥��
 *
 * @param pActionRad ��������
 *
 * @return ������������Ϊcount����������ʾȫ�����
 * @brief �����ֵ
 */
static uint8_t PulseOutTrigger(ActionRad* pActionRad)
{
	uint8_t i = 0;
	uint8_t count = pActionRad->count;
	ActionRad* pAction;



	for( i = 0; i < count; i++)
	{
		pAction = pActionRad+i;

		if (g_ProcessDelayTime[PHASE_A].calDelayCheck > 200000)
		{
			return 0xff;
		}

		switch(pAction->phase)
		{
			case PHASE_A:
			{

				EPwm2TimerInit( g_ProcessDelayTime[PHASE_A].calDelayCheck, TIME_BASE_4US);
				break;
			}
			case PHASE_B:
			{
				EPwm3TimerInit( g_ProcessDelayTime[PHASE_B].calDelayCheck, TIME_BASE_4US);
				break;
			}
			case PHASE_C:
			{
				EPwm4TimerInit( g_ProcessDelayTime[PHASE_C].calDelayCheck, TIME_BASE_4US);
				break;
			}
			default:
			{
				return 0xFE;
			}
		}
	}

	return 0;

}
#ifdef TEST_EXA_C

void pass_stop()
{
   __asm("   ESTOP0");
    for(;;);
}

void fail_stop()
{
   __asm("   ESTOP0");
    for(;;);
}

uint8_t test_result = 0;
float delayA = 0,delayB = 0,delayC = 0;
float calTimeA = 0,calTimeB = 0,calTimeC = 0;
uint8_t cn = 0, phase_cn = 0, period_cn = 0;;
uint32_t pass = 0,fail = 0;
float test_phase =0;
float diffA = 0,diffB = 0;

void TestCalculate(void)
{

	pass = 0;
	fail = 0;
	phase_cn = 0;

	do
	{
		delayA = 0.5*(100000 - 500 * cn);
		delayB = 40000;
		delayC = 0.5* (1000 + 500 * cn);


		g_SystemVoltageParameter.period = 20000;

		g_ProcessDelayTime[0].actionDelay = delayA;
		g_ProcessDelayTime[0].compensationTime = 0;
		g_ProcessDelayTime[0].sampleDelay = 0;
		g_ProcessDelayTime[0].transmitDelay = delayA;

		g_ProcessDelayTime[1].actionDelay = delayB;
		g_ProcessDelayTime[1].compensationTime = 0;
		g_ProcessDelayTime[1].sampleDelay = 0;
		g_ProcessDelayTime[1].transmitDelay = delayB;

		g_ProcessDelayTime[2].actionDelay = delayC;
		g_ProcessDelayTime[2].compensationTime = 0;
		g_ProcessDelayTime[2].sampleDelay = 0;
		g_ProcessDelayTime[2].transmitDelay = delayC;

		g_PhaseActionRad[0].actionRad = 0;
		g_PhaseActionRad[0].phase = 0;
		g_PhaseActionRad[0].count = 3;
		g_PhaseActionRad[0].enable = 0xFF;
		g_PhaseActionRad[0].realDiffRatio = 0;
		g_PhaseActionRad[0].realRatio = g_PhaseActionRad[0].realDiffRatio;

		g_PhaseActionRad[1].actionRad = 0;
		g_PhaseActionRad[1].phase = 1;
		g_PhaseActionRad[1].count = g_PhaseActionRad[0].count;
		g_PhaseActionRad[1].enable = 0xFF;
		g_PhaseActionRad[1].realDiffRatio = 0;
		g_PhaseActionRad[1].realRatio = g_PhaseActionRad[0].realRatio + g_PhaseActionRad[1].realDiffRatio;

		g_PhaseActionRad[2].actionRad = 0;
		g_PhaseActionRad[2].phase = 2;
		g_PhaseActionRad[2].count = g_PhaseActionRad[0].count;
		g_PhaseActionRad[2].enable = 0xFF;
		g_PhaseActionRad[2].realDiffRatio = 0;
		g_PhaseActionRad[2].realRatio = g_PhaseActionRad[1].realRatio + g_PhaseActionRad[2].realDiffRatio;

		CalculateDelayTime(g_PhaseActionRad, test_phase);
		CalculateDelayTime(g_PhaseActionRad + 1, test_phase);
		CalculateDelayTime(g_PhaseActionRad + 2, test_phase);




		test_result = CheckActionTime(g_PhaseActionRad);
		if (test_result != 0)
		{
			fail ++;
			 __asm("   ESTOP0");
			continue;
		}

		calTimeA = g_ProcessDelayTime[0].calDelayCheck + g_ProcessDelayTime[0].sumDelay;
		calTimeB = g_ProcessDelayTime[1].calDelayCheck + g_ProcessDelayTime[1].sumDelay;
		calTimeC = g_ProcessDelayTime[2].calDelayCheck + g_ProcessDelayTime[2].sumDelay;

		if ((fabsf(calTimeA - calTimeB) <= 3) && ((fabsf(calTimeB - calTimeC) <= 3)))
		{
			pass++;
			PulseOutTrigger(g_PhaseActionRad);
			while(1);
		}
		else
		{
			fail++;
			 __asm("   ESTOP0");
		}

	} while (cn++ < 1);
}
#endif


#ifdef TEST_EXA_B
void TestCalculate(void)
{

	pass = 0;
	fail = 0;
	period_cn = 0;
	do
	{
		phase_cn = 0;
		do
		{
			 test_phase = PI2*0.005*phase_cn; //��ͬ���
			 cn = 0;
			do
			{
				delayA = 0.5*(110000 - 500 * cn);
				delayB = 0.5*(210000 - 1000 * cn);
				delayC = 0.5* (1000 + 500 * cn);


				g_SystemVoltageParameter.period = 10000.3 + period_cn*100 ;

				g_ProcessDelayTime[0].actionDelay = delayA;
				g_ProcessDelayTime[0].compensationTime = 0;
				g_ProcessDelayTime[0].sampleDelay = 0;
				g_ProcessDelayTime[0].transmitDelay = delayA;

				g_ProcessDelayTime[1].actionDelay = delayB;
				g_ProcessDelayTime[1].compensationTime = 0;
				g_ProcessDelayTime[1].sampleDelay = 0;
				g_ProcessDelayTime[1].transmitDelay = delayB;

				g_ProcessDelayTime[2].actionDelay = delayC;
				g_ProcessDelayTime[2].compensationTime = 0;
				g_ProcessDelayTime[2].sampleDelay = 0;
				g_ProcessDelayTime[2].transmitDelay = delayC;

				g_PhaseActionRad[0].actionRad = 0;
				g_PhaseActionRad[0].phase = 0;
				g_PhaseActionRad[0].count = 3;
				g_PhaseActionRad[0].enable = 0xFF;
				g_PhaseActionRad[0].realDiffRatio = (float)phase_cn*327/65536;
				g_PhaseActionRad[0].realRatio =  g_PhaseActionRad[0].realDiffRatio;

				g_PhaseActionRad[1].actionRad = 0;
				g_PhaseActionRad[1].phase = 1;
				g_PhaseActionRad[1].count = 3;
				g_PhaseActionRad[1].enable = 0xFF;
				g_PhaseActionRad[1].realDiffRatio = (float)(65535 - g_PhaseActionRad[0].realRatio)/2/65536;
				g_PhaseActionRad[1].realRatio = g_PhaseActionRad[0].realRatio + g_PhaseActionRad[1].realDiffRatio;

				g_PhaseActionRad[2].actionRad = 0;
				g_PhaseActionRad[2].phase = 2;
				g_PhaseActionRad[2].count = 3;
				g_PhaseActionRad[2].enable = 0xFF;
				g_PhaseActionRad[2].realDiffRatio = (float)(65535 - g_PhaseActionRad[0].realRatio)/4/65536;
				g_PhaseActionRad[2].realRatio = g_PhaseActionRad[1].realRatio + g_PhaseActionRad[2].realDiffRatio;


				CalculateDelayTime(g_PhaseActionRad, test_phase);
				CalculateDelayTime(g_PhaseActionRad + 1, test_phase);
				CalculateDelayTime(g_PhaseActionRad + 2, test_phase);




				test_result = CheckActionTime(g_PhaseActionRad);
				if (test_result != 0)
				{
					fail ++;
					 __asm("   ESTOP0");
					continue;
				}

				calTimeA = g_ProcessDelayTime[0].calDelayCheck + g_ProcessDelayTime[0].sumDelay;
				calTimeB = g_ProcessDelayTime[1].calDelayCheck + g_ProcessDelayTime[1].sumDelay;
				calTimeC = g_ProcessDelayTime[2].calDelayCheck + g_ProcessDelayTime[2].sumDelay;

				diffA = fabsf(calTimeB - calTimeA - (g_PhaseActionRad[1].realTime  - g_PhaseActionRad[0].realTime));

				if (diffA <= 3)
				{
					pass++;
				}
				else
				{
					fail++;
					 __asm("   ESTOP0");
				}
				diffB = fabsf(calTimeC - calTimeB   - (g_PhaseActionRad[2].realTime - g_PhaseActionRad[1].realTime));
				if (diffB <= 3)
				{
					pass++;
				}
				else
				{
					fail++;
					 __asm("   ESTOP0");
				}

			} while (cn++ < 200);
		}while(phase_cn++ <200);
	}while(period_cn++ <200);
	 __asm("   ESTOP0");
}
#endif

#ifdef TEST_EXA_A
void TestCalculate(void)
{

	pass = 0;
	fail = 0;
	phase_cn = 0;
	do
	{
		 test_phase = PI2*0.005*phase_cn; //��ͬ���
		 cn = 0;
		do
		{
			delayA = 0.5*(100000 - 500 * cn);
			delayB = 40000;
			delayC = 0.5* (1000 + 500 * cn);


			g_SystemVoltageParameter.period = 19666;

			g_ProcessDelayTime[0].actionDelay = delayA;
			g_ProcessDelayTime[0].compensationTime = 0;
			g_ProcessDelayTime[0].sampleDelay = 0;
			g_ProcessDelayTime[0].transmitDelay = delayA;

			g_ProcessDelayTime[1].actionDelay = delayB;
			g_ProcessDelayTime[1].compensationTime = 0;
			g_ProcessDelayTime[1].sampleDelay = 0;
			g_ProcessDelayTime[1].transmitDelay = delayB;

			g_ProcessDelayTime[2].actionDelay = delayC;
			g_ProcessDelayTime[2].compensationTime = 0;
			g_ProcessDelayTime[2].sampleDelay = 0;
			g_ProcessDelayTime[2].transmitDelay = delayC;

			g_PhaseActionRad[0].actionRad = 0;
			g_PhaseActionRad[0].phase = 0;
			g_PhaseActionRad[0].count = 3;
			g_PhaseActionRad[0].enable = 0xFF;
			g_PhaseActionRad[0].realDiffRatio = 0;
			g_PhaseActionRad[0].realRatio = g_PhaseActionRad[0].realDiffRatio;

			g_PhaseActionRad[1].actionRad = 0;
			g_PhaseActionRad[1].phase = 1;
			g_PhaseActionRad[1].count = g_PhaseActionRad[0].count;
			g_PhaseActionRad[1].enable = 0xFF;
			g_PhaseActionRad[1].realDiffRatio = 0;
			g_PhaseActionRad[1].realRatio = g_PhaseActionRad[0].realRatio + g_PhaseActionRad[1].realDiffRatio;

			g_PhaseActionRad[2].actionRad = 0;
			g_PhaseActionRad[2].phase = 2;
			g_PhaseActionRad[2].count = g_PhaseActionRad[0].count;
			g_PhaseActionRad[2].enable = 0xFF;
			g_PhaseActionRad[2].realDiffRatio = 0;
			g_PhaseActionRad[2].realRatio = g_PhaseActionRad[1].realRatio + g_PhaseActionRad[2].realDiffRatio;


			CalculateDelayTime(g_PhaseActionRad, test_phase);
			CalculateDelayTime(g_PhaseActionRad + 1, test_phase);
			CalculateDelayTime(g_PhaseActionRad + 2, test_phase);




			test_result = CheckActionTime(g_PhaseActionRad);
			if (test_result != 0)
			{
				fail ++;
				 __asm("   ESTOP0");
				continue;
			}

			calTimeA = g_ProcessDelayTime[0].calDelayCheck + g_ProcessDelayTime[0].sumDelay;
			calTimeB = g_ProcessDelayTime[1].calDelayCheck + g_ProcessDelayTime[1].sumDelay;
			calTimeC = g_ProcessDelayTime[2].calDelayCheck + g_ProcessDelayTime[2].sumDelay;

			if ((fabsf(calTimeA - calTimeB) <= 1) && ((fabsf(calTimeB - calTimeC) <= 1)))
			{
				pass++;
			}
			else
			{
				fail++;
				 __asm("   ESTOP0");
			}


		} while (cn++ < 200);
	}
	while(phase_cn++ <200);
}
#endif
