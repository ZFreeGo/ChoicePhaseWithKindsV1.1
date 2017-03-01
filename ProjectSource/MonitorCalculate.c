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

#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "Header.h"


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


struct  FreqCollect FreqMonitor; //���Ƶ��
struct  FreqCollect* pFreqMonitor;//ָ����Ƶ��ָ��

volatile struct  TimeParameteCall CalTimeMonitor; //����ʱ�����
volatile struct  TimeParameteCall* pCalTimeMonitor;



volatile  struct  OrderParamCollect SetParam;// �趨����

volatile Uint8 FirstTrig = 0;// �״δ�����־ 0--����Ҫ���� ��0--��Ҫ����
/*=============================ȫ�ֱ������� End=============================*/

/*=============================���ñ��� extern Start=============================*/


/*=============================���ñ��� extern End=============================*/






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
       FreqMonitor, pFreqMonitor
       CalTimeMonitor, pFreqMonitor
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

  FreqMonitor.FreqInit = 50.0f;
  FreqMonitor.FreqReal = 50.0f;
  FreqMonitor.FreqMean = 50.0f;
  FreqMonitor.FreqCal = 50.0f;
  pFreqMonitor = &FreqMonitor;

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






/********************************************************************
 * ��������GetOVD()
 * ������
 * ����ֵ��NULL
 * ���ܣ����� FFT���������ݻ�ȡ����ʱ��
 ********************************************************************/
float phase = 0.0,a = 0, b = 0;
float tp = 0,tph = 0, tp4 = 0,tq = 0, t0 = 0, t1 = 0, sumt = 0;//��λʱ���
float tnow = 0; //��ǰʱ������
Uint16 xiang = 0;
Uint16 count = 0;
void GetOVD(float* pData)
{

	FFT_Cal(pData);
	a = RFFToutBuff[1]; //ʵ��
	b = RFFToutBuff[RFFT_SIZE - 1]; //�鲿
	phase = atan( b/a ); //��ȡ��λ
	//��λ�ж�
	if (phase >= -0.00001 ) //��Ϊ�ڵ�1,3����   �����������ж�����,�Ƿ���Ҫ���⴦��
	{
		if (a >= -0.00001)
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
	//���ޱ任
	switch (xiang)
	{
	case 1:
	{
		phase = phase;
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
	//����ֵ����
	tp = 1e6 / FreqMonitor.FreqReal; //��ʵ����
	tph = tp * 0.5f;  //�������


	//phase Ϊcosֵ cos
	/*
	 if (phase < PI3_2)// ʱ��t0; ������һ������ѭ���������ʱ��
	 {
	      //t0 = (3_2PI - phase)* tp * D2PI; //  1/2pi
		 t0 = (0.75f - phase * D2PI) * tp;
	 }
	 else
	 {
	      //t0 = (3_2PI - phase + 2*PI)* tp * D2PI;
		 t0 = (1.75f - phase * D2PI) * tp;
	 }
	*/
	//t0 = (2*pi - phase) * tp *D2PI;
	//t0 = (1.0f - phase * D2PI) * tp;

	//���趨��բ��ȡ
	SetParam.SetPhaseTime = SetParam.SetPhase * tp * D2PI; //��ȡ��Ӧ���ʱ��
	tnow = phase * tp * D2PI;
	sumt =   SetParam.SetPhaseTime - tnow - SetParam.HezhaTime;
	count = 2;
	do
	{
        t0 = count * tp + sumt;
        count ++;
	}
	while(t0 < 0);






#if WITH_FFT == 1

	// t1 = 10; //�ڲ���ʱ //�������������·��ʱ��Ӧ�ò����ڴˡ�
	t1 = 1713 + 10;
#elif WITH_ZVD == 1
	 t1 = 230;// %���뱾������ʼ����   ͨ�������ȡָ��������
#endif

	 if (t0 >= t1)
	{
		tq = t0 - t1 ;
	}
	else
	{
		tq =   t0 - t1 + tp; //%�ӳ�һ������
	}

	if (tq >= tp)
	{
		tq = tq - tp;
	}

	CalTimeMonitor.CalTimeDiff = tq; //��ֵ��ʱ��

	TOGGLE_LED1; //���������־


	DELAY_US(tq); //��ʱ����
#if WITH_FFT == 1

	DELAY_US(93); //1.����������ʱ���ͺ�ʱ�� �� 2.���ڴ���ͬ����ʱ��Ӧ���ڴ˴���ȥ.140->80

#elif WITH_ZVD == 1


#endif


	//��ʱΪ��ʵ�ʹ���㿪ʼ����
	//��Ŀ��Ϊ����ʵ�ʼ�������

	FirstTrig = 0xff;// ׼���״δ���
	InitCpuTimers();


	ConfigCpuTimer(&CpuTimer0, 80, tph);
	//������ʱ��
	//SET_YONGCI_ACTION(); //ͬ����բ��ʱ��
	TOGGLE_LED2; //�״δ�������
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	//��ֵ�Ա�����
	CalTimeMonitor.CalTp = tp;
	CalTimeMonitor.CalT0 = t0;
	CalTimeMonitor.CalPhase = phase;
	pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //��Ϊ������־

}


__interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;
   TOGGLE_LED2;
   TOGGLE_LED1;
   TOGGLE_LED3;
   if (FirstTrig)
   {
	   FirstTrig = 0;
	  // RESET_YONGCI_ACTION();
   }
   // Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
