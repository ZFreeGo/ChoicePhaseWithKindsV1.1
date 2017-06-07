/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Main.c --- 此方法基于FFT
*文件标识:
*创建日期： 2015年1月28日
*摘要:
*详 见程序所附说明文档.txt
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/

#include "DSP28x_Project.h"
#include "Header.h"
#include "RefParameter.h"

#define FLASH   1
#ifdef FLASH
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;
#endif

/*=============================全局变量定义 Start=============================*/


//测频算法区域
//float FS = 3200; //采样率
/*=============================全局变量定义 End=============================*/




int main(void)
{
	uint32_t cnTime = 0;
	InitSysCtrl();            //20*4 = 80M
	InitDeviceIO();

	// Step 3. Clear all interrupts and initialize PIE vector table:
	DINT;

	InitPieCtrl();
	IER = 0x0000;
	IFR = 0x0000;
	InitPieVectTable();
	EALLOW;
	// This is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = &Cpu_timer0_isr;
	PieVectTable.ADCINT1 = &ADC_INT1_ISR;
	//PieVectTable.ADCINT2 = &ADC_INT2_Potect_ISR;
	//PieVectTable.SCIRXINTB = &ScibRX_ISR;
	//PieVectTable.SCITXINTB = &ScibTX_ISR;
	//PieVectTable.SCIRXINTA = &SciaRX_ISR;
	//PieVectTable.SCITXINTA = &SciaTX_ISR;
	PieVectTable.EPWM2_INT = &epwm2_timer_isr;
	PieVectTable.EPWM3_INT = &epwm3_timer_isr;
	PieVectTable.EPWM4_INT = &epwm4_timer_isr;
	PieVectTable.ECAN0INTA = &Can0Recive_ISR;
	PieVectTable.ECAP2_INT = &ecap2_isr;
	EDIS;
	// This is needed to disable write to EALLOW protected registers

	//针对Flash模式，将关键算法程序从Flash拷贝到RAM中运行
#ifdef FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32) &RamfuncsLoadSize);
	InitFlash();
#endif

	//Step 4. Initialize the Device Peripheral.


	InitStandardCAN(0, 0);
	InitAdc(); //初始化ADC
	InitCpuTimers(); //初始化CPU寄存器
	ConfigCpuTimer(&CpuTimer0, 80, 1000); //配置CPU在80M工作频率下，中断周期1000us
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0 启动定时器

	//使能外PIE中断向量
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1; //TIMER0 	// Enable TINT0 in the PIE: Group 1 interrupt 7

	PieCtrlRegs.PIEIER1.bit.INTx1 = 1; //ADCINT1
	//PieCtrlRegs.PIEIER1.bit.INTx2 = 1;//ADCINT2  Group 1 interrupt 2
	PieCtrlRegs.PIEIER3.bit.INTx4 = PWM4_INT_ENABLE;
	PieCtrlRegs.PIEIER9.bit.INTx5 = 1; //ECAN0bits
	PieCtrlRegs.PIEIER4.bit.INTx2 = 1; //ECAP2


	PieCtrlRegs.PIEIER3.bit.INTx2 = PWM2_INT_ENABLE;
	PieCtrlRegs.PIEIER3.bit.INTx3 = PWM3_INT_ENABLE;


	IER |= M_INT1;		//TIMER0 ADCINT1 ADCINT2
	IER |= M_INT3;        // Enable CPU INT3 which is connected to EPWM1-6 INT:
	IER |= M_INT9;
	IER |= M_INT4; //ECAP2
	EINT;
	// Enable Global interrupt INTM
	ERTM;
	// Enable Global realtime interrupt DBGM

	InitSampleProcessData(); //采样数据初始化
	InitMonitorCalData(); //监控数据计算初始化
	ConfigADC_Monitor(12500);  //ADC 采样初始化 设定采样周期 12500属于定时器计数长度，每周波64点
	ConfigECapFrquency();
	ActionInit();
	InitDeviceNet();

	//InitEPwmTimer();
	//调试使用
	StartSample();//用于启动采样



	while (1)
	{
		 UpdateFrequency();
		 AckMsgService();
		 if(cnTime++ > 200000)
		 {
			 TOGGLE_LED1;
			 cnTime = 0;
		 }
		 //TestCalculate();

	}

	

}


