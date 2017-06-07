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
#include "DSP28x_Project.h"
#include "F2806x_Examples.h"   // F2806x Examples Include File
#include "Header.h"
#include "RefParameter.h"
#include "Action.h"

#include <math.h>
#include <stdlib.h>
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


//struct  FreqCollect  g_SystemVoltageParameter.frequencyCollect; //监控频率


volatile struct  TimeParameteCall CalTimeMonitor; //计算时间过程
volatile struct  TimeParameteCall* pCalTimeMonitor;



volatile  struct  OrderParamCollect SetParam;// 设定参数

volatile Uint8 FirstTrig = 0;// 首次触发标志 0--不需要触发 非0--需要触发
/*=============================全局变量定义 End=============================*/

/*=============================局部函数 Start=============================*/
static uint8_t GetMaxActionTime(ActionRad* pActionRad);
static int8_t GetTimeDiff(float sumTime1, float sumTime2, float period, float diff);
static uint8_t PulseOutTrigger(ActionRad* pActionRad);
static uint8_t CheckActionTime(ActionRad* pActionRad);
static uint8_t CalculateDelayTime(ActionRad* pActionRad, float phase);
/*=============================局部函数 End=============================*/



#define N_MAX  20 //最大计算次数
#define ERROR_VALUE 10//单周期误差 10us

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
  //TwoDivideN = 1;//TwoDivideN = 2.0 / (float)RFFT_SIZE; 消去
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
  SetParam.SetPhase = 0; //此处以COS标准 弧度
  SetParam.SetPhaseTime = 20000 * 0.75f;
  SetParam.HeFenFlag = 0;//合闸

  RefParameterInit();
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

/**
 * 使用FFT值计算有效值
 */
void CalEffectiveValue(void)
{
	uint16_t j = 0;
	float sum = 0;
	float basic = RFFToutBuff[0]/RFFT_SIZE;
	 //计算实部与虚部平方和作为有效值 基波
	 // g_SystemVoltageParameter.voltageA = sqrt( RFFToutBuff[1]*RFFToutBuff[1] + RFFTmagBuff[1]*RFFTmagBuff[1]) * g_SystemCalibrationCoefficient.voltageCoefficient1;
	  for(j=0;j<RFFT_SIZE;j++)
	  {
		  sum += (RFFTin1Buff[j] -  basic) * (RFFTin1Buff[j] -  basic); //
	  }
	  g_SystemVoltageParameter.voltageA = sqrt(sum/RFFT_SIZE) *  g_SystemCalibrationCoefficient.voltageCoefficient1;


}

/**
 * 同步触发器，利用采样数据计算同步触发动作时刻，发出触发命令,以A相为例
 *
 * @param  *pData   指向采样数据的指针
 * @brief  计算触发时刻，发布触发命令
 */
void SynchronizTrigger(float* pData)
{
	float phase = 0.0,a = 0, b = 0;
	uint16_t xiang = 0, i = 0;
	uint16_t  test_result = 0;
	float   calTimeA = 0, calTimeB = 0,diffA = 0;
	FFT_Cal(pData);   //傅里叶变化计算相角 每个点间隔
	a = RFFToutBuff[1]; //实部
	b = RFFToutBuff[RFFT_SIZE - 1]; //虚部
	phase = atan( b/a ); //求取相位(-pi/2 pi/2)


	//相位判断
		if (phase >= 0 ) //认为在第1,3象限   浮点数与零判断问题,是否需要特殊处理？
		{
			if (a >= 0)
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
		phase += PID2;//cos转化为sin
		if (phase > PI2)//大于2PI
		{
			phase = phase - PI2;
		}

		//象限变换
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
		//算错误报错
		if (test_result != 0)
		{
			SynActionAck(0xA1);
			return;
		}

		//计算时间差
		for(i = 1; i < g_PhaseActionRad->count; i++)
		{
			calTimeA = g_ProcessDelayTime[0].calDelayCheck + g_ProcessDelayTime[0].sumDelay;
			calTimeB = g_ProcessDelayTime[1].calDelayCheck + g_ProcessDelayTime[1].sumDelay;
			diffA = fabsf(calTimeB - calTimeA - (g_PhaseActionRad[1].realTime  - g_PhaseActionRad[0].realTime));
			if (diffA > 3)
			{
				SynActionAck(0xA2);
				return;//校验错误
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
 * 计算动作延时时间
 *
 * @param  pActionRad   动作弧度
 * @param  phase	    开始相角
 *
 * @return 0-正常   非0-错误
 * @brief  计算触发时刻，发布触发命令
 */
static uint8_t CalculateDelayTime(ActionRad* pActionRad, float phase)
{

	uint16_t count = 0;
	uint8_t selectPhase = 0;
	float time = 0, difftime = 0;

	//未使能跳出
	if (pActionRad->enable == 0)
	{
		return 0;
	}
	//phase 小于等于3
	if ( pActionRad->phase >= 3)
	{
		return 0xF1;
	}
	//TODO:暂定内部延时为88us
	g_ProcessDelayTime[selectPhase].innerDelay = 88;
	selectPhase = pActionRad->phase;

	//计算开始时间
	pActionRad->startTime = g_SystemVoltageParameter.period * phase * D2PI;
	//此处相乘，为了保证使用最新的周期
	pActionRad->realTime = g_SystemVoltageParameter.period
			* pActionRad->realRatio;
	pActionRad->realDiffTime = g_SystemVoltageParameter.period
			* pActionRad->realDiffRatio;
	//计算延时之和
	g_ProcessDelayTime[selectPhase].sumDelay =
			(float)g_ProcessDelayTime[selectPhase].sampleDelay + (float)g_ProcessDelayTime[selectPhase].pulseDelay
					+ (float)g_ProcessDelayTime[selectPhase].transmitDelay
					+ (float)g_ProcessDelayTime[selectPhase].actionDelay
					+ (float)g_ProcessDelayTime[selectPhase].innerDelay;
	//总的时间和
	time = (float)g_ProcessDelayTime[selectPhase].sumDelay + (float)pActionRad->startTime
			- (float)pActionRad->realTime;

	count = 1;
	do
	{
		difftime = count *  g_SystemVoltageParameter.period - time;
		//判断时间差是否大于0，若大于则跳出
		if(difftime > 0)
		{
			break;
		}
		count++;

	}while(count < 100);
	//添加异常判断
	if (count == 100)
	{
		return 0xF1;
	}



	if (difftime > 0)
	{
		//若时间之和大于等于0，则正常补偿；否则添加一个周期的延时
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
				g_ProcessDelayTime[selectPhase].calDelay = difftime;//超限后不进行补偿。
			}


		}
		//默认赋值
		g_ProcessDelayTime[selectPhase].calDelayCheck = g_ProcessDelayTime[selectPhase].calDelay;
		return 0;
	}
	else
	{
		//异常处理
		return 0xF2;
	}
}

/**
 * 校准计算动作时间
 *
 * @param  pActionRad   指向动作时序命令
 *
 * @return 0-计算正常 非0--有错误出现
 * @brief  计算触发时刻，发布触发命令
 */


static uint8_t CheckActionTime(ActionRad* pActionRad)
{
	uint8_t maxIndex = 0;
	int8_t diff_result = 0;
	uint8_t  i = 0,select1 = 0, select2 = 0;
	float t1 = 0, t2 = 0;
	//获取最大索引
	maxIndex = GetMaxActionTime(pActionRad);
			//若相等或者count=1，表示不需要校准
	if ((maxIndex == pActionRad->count) || ( pActionRad->count == 1) )
	{
		return 0;
	}

	//小于maxIndex索引进行校准
	for ( i = maxIndex; i > 0; i--)
	{
		select1 = pActionRad[i-1].phase;
		t1 = g_ProcessDelayTime[select1].sumDelay + g_ProcessDelayTime[select1].calDelayCheck ;//添加校准后的数据
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
	//大于maxIndex索引进行校准
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
				g_ProcessDelayTime[select1].calDelayCheck = g_ProcessDelayTime[select1].calDelay;//直接赋值
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
 * 计算时间差值
 *
 * @param sumTime1 累加计时1
 * @param sumTime2 累加计时2
 * @param period   周期
 * @param diff     差值
 *
 * @return  n = N_MAX --计算错误
		   n = 0—当前值不需要变动
		   n > 0----SumTime1需要增加n*period
		   n <0 ----SumTime2需要增加n*period
		   
   @brief 计算差值
 */

static int8_t GetTimeDiff(float sumTime1, float sumTime2, float period, float diff)
{
	int8_t  n = 0;//循环计数
	float time = 0;
	n = 0;
    if((sumTime1 - sumTime2 + diff) > ERROR_VALUE)
    {
    	//T1 大于T2，T2需要周期数递增
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
		//T2 大于T1，T1需要周期数递增
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
 * 计算时间差值
 *
 * @param pActionRad 动作弧度
 *
 * @return 最大的索引，若为count（数量）表示全部相等
 * @brief 计算差值
 */
static uint8_t GetMaxActionTime(ActionRad* pActionRad)
{
	uint8_t i = 0;
	uint8_t count = pActionRad->count;
	uint8_t maxIndex = 0, maxFlag = 0;
	uint8_t selectPhase = 0;
	float sum = 0, maxSum = 0;


	selectPhase = pActionRad[0].phase;
	//时间之和=总延时 + 计算延时 + 相对于最后一个的提前时间
	maxSum = g_ProcessDelayTime[selectPhase].sumDelay + g_ProcessDelayTime[selectPhase].calDelay + pActionRad[count - 1].realTime - pActionRad[0].realTime;

	for (i = 1; i < count; i++)
	{
		selectPhase = pActionRad[i].phase;
		//计算总时间
		sum = g_ProcessDelayTime[selectPhase].sumDelay + g_ProcessDelayTime[selectPhase].calDelay + pActionRad[count - 1].realTime - pActionRad[i].realTime;
		//是否大于MaxSum
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
 * 同步输出脉冲触发
 *
 * @param pActionRad 动作参数
 *
 * @return 最大的索引，若为count（数量）表示全部相等
 * @brief 计算差值
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
			 test_phase = PI2*0.005*phase_cn; //不同相角
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
		 test_phase = PI2*0.005*phase_cn; //不同相角
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
