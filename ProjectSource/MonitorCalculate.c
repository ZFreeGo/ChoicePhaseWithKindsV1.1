/****************************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:MonitorCalculate.c
*文件标识:
*创建日期： 2015年1月25日
*摘要:
*2105/3/24: 改综合变量为结构体
*2015/1/31: 添加针对ADC数据的处理流程
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
*******************************************************/

#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "Header.h"


/*=============================全局变量定义 Start=============================*/
//#define RFFT_STAGES 6
//#define RFFT_SIZE (1 << RFFT_STAGES)
/*FFTin1Buff section to 2*FFT_SIZE in the linker file*/

//FFT变换，变量内存分配
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
RFFT_F32_STRUCT rfft; //FFT 变换数据结构体

//正弦余弦系数
//float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
//float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
//float TwoDivideN = 0; //2/N


struct  FreqCollect FreqMonitor; //监控频率
struct  FreqCollect* pFreqMonitor;//指向监控频率指针

volatile struct  TimeParameteCall CalTimeMonitor; //计算时间过程
volatile struct  TimeParameteCall* pCalTimeMonitor;



volatile  struct  OrderParamCollect SetParam;// 设定参数

volatile Uint8 FirstTrig = 0;// 首次触发标志 0--不需要触发 非0--需要触发
/*=============================全局变量定义 End=============================*/

/*=============================引用变量 extern Start=============================*/


/*=============================引用变量 extern End=============================*/






/**************************************************
 *函数名：InitMonitorCalData ()
 *功能： 初始化FFT模块，初始化变量，RFFT
 *形参：void
 *返回值：void
 *调用函数：FFT_Init()
 *引用外部变量：
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
  //TwoDivideN = 1;//TwoDivideN = 2.0 / (float)RFFT_SIZE; 消去
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
  SetParam.SetPhase = 0; //此处以COS标准 弧度
  SetParam.SetPhaseTime = 20000 * 0.75f;
  SetParam.HeFenFlag = 0;//合闸
}
/**************************************************
 *函数名：FFT_Init()
 *功能： 初始化FFT 基本数据
 *形参：
 *返回值：void
 *调用函数： null
 *引用外部变量： rfft
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
 *函数名：FFT_Cal ()
 *功能： FFT计算
 *形参：float ADsample[] -- ADC采样值
 *返回值：void
 *调用函数：RFFT_f32_sincostable(), RFFT_f32()
 *引用外部变量： RFFTin1Buff[], rfft
****************************************************/
void FFT_Cal(float ADsample[])
{

	Uint16 j=0;
	//将数据复制与此
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
 * 函数名：GetOVD()
 * 参数：
 * 返回值：NULL
 * 功能：基于 FFT，根据数据获取过零时刻
 ********************************************************************/
float phase = 0.0,a = 0, b = 0;
float tp = 0,tph = 0, tp4 = 0,tq = 0, t0 = 0, t1 = 0, sumt = 0;//相位时间差
float tnow = 0; //当前时间折算
Uint16 xiang = 0;
Uint16 count = 0;
void GetOVD(float* pData)
{

	FFT_Cal(pData);
	a = RFFToutBuff[1]; //实部
	b = RFFToutBuff[RFFT_SIZE - 1]; //虚部
	phase = atan( b/a ); //求取相位
	//相位判断
	if (phase >= -0.00001 ) //认为在第1,3象限   浮点数与零判断问题,是否需要特殊处理？
	{
		if (a >= -0.00001)
		{
			xiang = 1; //在第一象限

		}
		else
		{
			xiang = 3; //在第三象限
		}
	}
	else
	{
		if (a >= 0)
		{
			xiang = 4; //在第四象限
		}
		else
		{
			xiang = 2; //在第二象限
		}
	}
	//象限变换
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
	//超阈值处理
	tp = 1e6 / FreqMonitor.FreqReal; //真实周期
	tph = tp * 0.5f;  //半个周期


	//phase 为cos值 cos
	/*
	 if (phase < PI3_2)// 时间t0; 距离下一个周期循环正过零点时间
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

	//按设定合闸求取
	SetParam.SetPhaseTime = SetParam.SetPhase * tp * D2PI; //求取对应相角时间
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

	// t1 = 10; //内部延时 //修正采样调理电路延时，应该补偿于此。
	t1 = 1713 + 10;
#elif WITH_ZVD == 1
	 t1 = 230;// %调入本函数开始进程   通过仿真获取指令周期数
#endif

	 if (t0 >= t1)
	{
		tq = t0 - t1 ;
	}
	else
	{
		tq =   t0 - t1 + tp; //%延迟一个周期
	}

	if (tq >= tp)
	{
		tq = tq - tp;
	}

	CalTimeMonitor.CalTimeDiff = tq; //赋值给时间差。

	TOGGLE_LED1; //采样输出标志


	DELAY_US(tq); //延时代替
#if WITH_FFT == 1

	DELAY_US(93); //1.修正固有延时，滞后时间 。 2.对于传输同步延时，应该在此处减去.140->80

#elif WITH_ZVD == 1


#endif


	//延时为到实际过零点开始计算
	//此目的为补偿实际计算等误差

	FirstTrig = 0xff;// 准备首次触发
	InitCpuTimers();


	ConfigCpuTimer(&CpuTimer0, 80, tph);
	//启动定时器
	//SET_YONGCI_ACTION(); //同步合闸定时器
	TOGGLE_LED2; //首次触发跳变
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	//赋值以备调用
	CalTimeMonitor.CalTp = tp;
	CalTimeMonitor.CalT0 = t0;
	CalTimeMonitor.CalPhase = phase;
	pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //置为非满标志

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
