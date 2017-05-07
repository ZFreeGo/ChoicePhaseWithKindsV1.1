/****************************************************
*Copyright(c) 2016,FreeGo
*保留所有权利
*文件名称:FrequrncyCaluate.c
*文件标识:
*创建日期： 2016年11月11日
*摘要:
*2016年11月11日: 将测频程序独立成文件，增加模块化.
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
*******************************************************/
#include "FrequrncyCaluate.h"
#include "Header.h"
#include "RefParameter.h"

/*=============================全局变量定义 Start=============================*/
//正弦余弦系数
float Cos1step = 0;  //cos(2*PI*step/RFFT_SIZE)
float Sin1step = 0;  //sin(2*PI*step/RFFT_SIZE)
float TwoDivideN = 0; //2/N
/*=============================全局变量定义 End=============================*/

void CaliAB_Base(float yangben[], AngleElement* pBase);
void GetNewFreq(float yangben[], float f0, float* pFq);
void CalFreq(float* pData);

float  MidMeanFilter(float *pData, Uint8 len);


/**************************************************
 *函数名：CaliAB_Base()
 *功能： 计算存在直流衰减分量时的补偿值，并计算实时相位值
 *形参：float yangben[] 样本值，可以是直接ADC或者滤波后的值。AngleElement* pBase 数据点初相角相位
 *返回值：void
 *调用函数： atan()
 *引用外部变量：TwoDivideN, Cos1step
****************************************************/
void CaliAB_Base(float yangben[],AngleElement* pBase)
{
	float DA;  //为了跟踪调试
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
        fzSum += yangben[2*i + 1]; //1,3,  奇数索引项求和，对应数据偶数项
        fmSum += yangben[2*i];//0,2,4

      }
    B = fzSum / fmSum;  //求B阵
    //所有项求和
    sum = fzSum + fmSum;
    /*
    for (i = 0; i < RFFT_SIZE; i++)
     {
        sum += yangben[i];
      }
      */
    A = sum * (1-B);  //求系数A

    //计算校正值  获取基波相位
    float  fm = 1,  fmrate = 1;
    //fm = 1 - 2 * B * Cos1step + B * B;
    fm = 1 - (2 * Cos1step - B) * B;
    fmrate = 1.0 / fm;
    DA = TwoDivideN * A * (1 - B * Cos1step) * fmrate; //可考虑消去TwoDivideN
    DB = TwoDivideN * A * B * Sin1step * fmrate;

    calRel = RFFToutBuff[1] * TwoDivideN - DA; //基波实部补偿
    calImag = RFFToutBuff[RFFT_SIZE - 1]*TwoDivideN + DB; //基波虚部补偿

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
 * 计算修正后的频率
 *
 * @param  yangben   两个周期的样本值
 * @param  f0        当前频率
 * @param  *pFq      修正后的频率指针
 * @brief  利用历史数据校正频率
 */
void GetNewFreq(float yangben[], float f0, float* pFq)
{
	float df1 = 0, r = 0, dA = 0;
  AngleElement baseA, baseB; //保存中间信息

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
  //dA修正   实际过程中不可能偏差大于PI
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
 * 计算频率模块，根据数据计算频率
 *
 * @param  pData   指向处理数据的指针，需要的2*N个数据，可以是直接数据或处理后的数据
 *
 * @brief   利用历史数据校正频率
 */
void CalFreq(float* pData)
{
  if (pData[SAMPLE_LEN] == SAMPLE_COMPLTE) //已经完成采样
    {

      GetNewFreq(pData, g_SystemVoltageParameter.frequencyCollect.FreqReal, &g_SystemVoltageParameter.frequencyCollect.FreqCal); //求取新频率
      pData[SAMPLE_LEN] = SAMPLE_NOT_FULL; //置为非满标志

      //判断新求得的频率是否在阈值内，如果不在区域内，则置频率为默认频率，并报错
      if ((g_SystemVoltageParameter.frequencyCollect.FreqCal >=  g_SystemLimit.frequency.min)
    		  && (g_SystemVoltageParameter.frequencyCollect.FreqCal <=  g_SystemLimit.frequency.max))
        {
          g_SystemVoltageParameter.frequencyCollect.FreqReal = g_SystemVoltageParameter.frequencyCollect.FreqCal; //置新频率为默认频率  //
        }
      else
        {
          g_SystemVoltageParameter.frequencyCollect.FreqReal = g_SystemVoltageParameter.frequencyCollect.FreqInit; //置为默认频率
          //发送错误信息
        }
    }
}


/**
 * 中值数字滤波模块
 *
 * @param  pData   指向带滤波的数组
 * @param  len     数据长度
 * @return         平均滤波结果
 * @brief   中值平均滤波,去掉最小值与最大值求取平均值
 */
float  MidMeanFilter(float *pData, Uint8 len)
{
	float min = 0, max = 0, sum = 0;
	float res = 0; //最终结果
	Uint8 i = 0;
	min = pData[0];
	max=  pData[0];

	for (i = 0; i < len; i++)
	{
		sum += pData[i]; //求和
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
 * 更新计算频率
 * <p>
 * 以float形式存储，以4字节进行交换传输的数据
 * @param   null
 *
 * @brif 用于通讯交互数据使用
 */
void UpdateFrequency(void)
{
	float samplePriod = 0;
	//测频模块处理
			//理论上将是20ms一个循环
	//Todo:浮点数等于比较是个隐患
	if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_COMPLTE) //采样完成
	{
		//计算频率
		CalFreq(SampleDataSavefloat);
		CalEffectiveValue();
		if (freqLen < 7) //计数长度 每7求取平均值计算周期
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
		samplePriod = 15625.0f / g_SystemVoltageParameter.frequencyCollect.FreqMean; //计算实时采样周期 1e6/64

		SetSamplePriod(samplePriod);
		StartSample(); //继续开始采样，再次测频
	}

}

