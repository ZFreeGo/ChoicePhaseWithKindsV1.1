/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:ConfigADC.c
*文件标识:
*创建日期： 2015年1月31日
*摘要:
*2015/1/31: ADC采样配置等等
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include"ConfigADC.h"
#include"DSP28x_Project.h"
#include "Header.h"


/*=============================全局变量定义 Start=============================*/
/*=============================全局变量定义 End=============================*/

/*=============================引用变量 extern Start=============================*/
extern Uint16 SampleIndex; //采样索引 from SampleProcess.c
/*=============================引用变量 extern End=============================*/


/********************************************************************
 * 函数名：ConfigADC_Monitor()
 * 参数：NULL
 * 返回值：NULL
 * 功能：配置监控首先配置 每周波64点  ADCINA0 电压采样
 *调用函数： null
 *引用外部变量： null
 ********************************************************************/
void ConfigADC_Monitor(float priod)
{
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP   = 1;	// Enable non-overlap mode
    AdcRegs.ADCCTL1.bit.INTPULSEPOS	= 1;	// ADCINT1 trips after AdcResults latch

    AdcRegs.INTSEL1N2.bit.INT1E         = 1;	// Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT      = 0;	// Disable ADCINT1 Continuous mode
    AdcRegs.INTSEL1N2.bit.INT1SEL 	= 0;    // setup EOC0 to trigger ADCINT1 to fire

    AdcRegs.ADCSOC0CTL.bit.CHSEL 	= 0x0C;    // set SOC0 channel select to ADCINB4


    AdcRegs.ADCSOC0CTL.bit.TRIGSEL 	= 5;    // set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1

    AdcRegs.ADCSOC0CTL.bit.ACQPS 	= 16;	// set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)

    EDIS;
    //周期设置
    EPwm1Regs.ETSEL.bit.SOCAEN	= 1;		// Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL	= 4;		// Select SOC from CMPA on upcount
    EPwm1Regs.ETPS.bit.SOCAPRD 	= 1;		// Generate pulse on 1st event
    EPwm1Regs.CMPA.half.CMPA 	= 0;	// 20*10^3/64*80/2 = 12500  default /2
    EPwm1Regs.TBPRD 			= priod;	// 12500  * FREQ_CALI_RATE// Set period for ePWM1   (TBPRD-CMPA)*2 宽度
    //EPwm1Regs.TBCTL.bit.CTRMODE = 0;		// count up and start
}

/********************************************************************
 * 函数名：StartSample()
 * 参数：void
 * 返回值：void
 * 功能：初始化采样索引，启动采样
 *调用函数： null
 *引用外部变量： null
 ********************************************************************/
void StartSample(void)
{
	SampleIndex = 0;
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;            // count up and start
}

/********************************************************************
 * 函数名：StopSample()
 * 参数：NULL
 * 返回值：NULL
 * 功能：停止采样
 * 调用函数： null
 * 引用外部变量： null
 ********************************************************************/
void StopSample(void)
{
	EPwm1Regs.TBCTL.bit.CTRMODE = 0b11;            // count up and start
}

/********************************************************************
 * 函数名：SetSamplePriod()
 * 参数：NULL
 * 返回值：NULL
 * 功能：设置采样周期,单位us  系统指令时钟80M us<1638us
 * 调用函数： null
 * 引用外部变量： null
 ********************************************************************/
void SetSamplePriod(float us)
{
  //停止计时
  EPwm1Regs.TBCTL.bit.CTRMODE = 0b11;
  //周期设置
  EPwm1Regs.ETSEL.bit.SOCAEN  = 1;            // Enable SOC on A group
  EPwm1Regs.ETSEL.bit.SOCASEL = 4;            // Select SOC from CMPA on upcount
  EPwm1Regs.ETPS.bit.SOCAPRD  = 1;            // Generate pulse on 1st event
  EPwm1Regs.CMPA.half.CMPA    = 0;    // 20*10^3/64*80/2 = 12500  default /2
  EPwm1Regs.TBPRD                     = (Uint16)(us * 40.0 * OVER_SAMPLE_RATE * FREQ_CALI_RATE);        // Set period for ePWM1   (TBPRD-CMPA)*2 宽度
 // EPwm1Regs.TBCTL.bit.CTRMODE = 0;            // count up and start

}


