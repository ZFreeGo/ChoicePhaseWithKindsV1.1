/***********************************************
*Copyright(c) 2011,FreeGo
*保留所有权利
*文件名称:SampleProcess.c
*文件标识:
*创建日期： 2011年1月1日
*摘要:
*上午11:05:53:创建本文件
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/

#include "SampleProcess.h"
#include "MonitorCalculate.h"
#include "DSP28x_Project.h"


/*=============================全局变量定义 Start=============================*/
Uint16 SampleData[SAMPLE_LEN] = {0}; //采样数据存储
Uint16 SampleDataSave[SAMPLE_LEN + 1] = {0}; //采样数据存储 转存 (SAMPLE_LEN + 1)长度
//Uint16 到 float 变换，后期可以考虑重用空间，减少ram使用
float SampleDataSavefloat[SAMPLE_LEN + 1] = {0};


Uint16 SampleIndex = 0; //采样索引
Uint16 i = 0;

Uint16 SampleLen = SAMPLE_LEN;
float SamplePriod = 0;
/*=============================全局变量定义 End=============================*/

/*=============================引用变量 Start=============================*/



/**
 * 开始同步触发计算标志
 */
volatile Uint8  ZVDFlag = 0;
/*=============================引用变量 End=============================*/



/********************************************************************
 * 函数名：InitSampleProcessData()
 * 参数：NULL
 * 返回值：NULL
 * 功能：初始化采样数据
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
  SamplePriod =  	15625.0f /  g_SystemVoltageParameter.frequencyCollect.FreqMean; //设置新频率  50hz
#elif  WITH_ZVD == 1
  SamplePriod = 156.25;//7812.5/50 恢复原有周期

#endif

}




/********************************************************************
 * 函数名：ADC_INT1_ISR()
 * 参数：NULL
 * 返回值：NULL
 * 功能：中断函数，ADCINT1中断
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
	   TOGGLE_LED1; //采样输出标志
   }
   if (SampleIndex >= SampleLen)
     {
       StopSample(); //停止采样
       SampleIndex = 0;

       TOGGLE_LED1; //采样输出标志

       //判断数据是否已经处理   若没有处理 则不进行数据转存，丢弃本次数据.  浮点数据计算
       if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_NOT_FULL)
         {
           for (i = 0; i < SAMPLE_LEN; i++)
             {
                 SampleDataSave[i] = SampleData[i]; //转存数据
                 SampleDataSavefloat[i] = SampleData[i];
             }
           SampleDataSave[SAMPLE_LEN] = SAMPLE_COMPLTE; //最后一个置为采样存储完成标志
           SampleDataSavefloat[SAMPLE_LEN] = SAMPLE_COMPLTE;


           //计算时间差
           if (ZVDFlag) //如果开始计算
           {
        		ConfigCpuTimer(&CpuTimer1, 80, 10000); //配置CPU在80M工作频率下，最大计时10ms
        		//TODO:停止定时器，防止打断中断，凭借优先级设置
        		CpuTimer0Regs.TCR.bit.TSS = 1;
        		CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0 启动定时器
#if WITH_FFT == 1
        	   ZVDFlag = 0; //首先清空标志，防止重复进入

        	   //GetOVD(SampleDataSavefloat); //7825 7803cyc 80 10us
        	   SynchronizTrigger(SampleDataSavefloat);


        	   SampleLen = SAMPLE_LEN; //原始
        	   //最后恢复状态
        	   StartSample();

#elif  WITH_ZVD == 1
        	   ZVDFlag = 0; //首先清空标志，防止重复进入
        	   GetOVD(SampleDataSavefloat); //7825 7803cyc 80 10us //同上

        	   //添加获取处理流程
        	   SampleLen = SAMPLE_LEN; //原始
        	   SamplePriod = 156.25; //恢复原有采样周期
#elif
#error "为适当定义当前采样模式"
#endif

        	   //重新启动timer0
        	    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0 启动定时器
           }
         }
       else
         {
            //发送未转存状态指示
         }

     }


   return;
}
