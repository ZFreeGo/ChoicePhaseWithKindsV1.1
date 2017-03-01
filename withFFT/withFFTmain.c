/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Main.c --- 此方法基于FFT
*文件标识:
*创建日期： 2015年1月28日
*摘要:
*摘要:
*2016/11/9:
*
*2016/11/9: 源文件复制，改版本为V1.1 进行适应性修改。
*           满足C语言约定，具体有如下修改：修改 frameRtu 为FrameRtu
*           删除串口B程序,
*
*
*
*2015/6/29:添加合闸相角设置，包括合闸角度，合闸时间等参数。
*2015/6/18:添加接收超时处理，添加等待同步处理。后发现超时处理会影响采样定时器，因此又取消，以后应该加上，避免干涉。
*修改lib文件参考，使其能够适应新版本，通过FLASH编程的memcpy运行。
*2015/6/15: 添加485通讯程序，完成两路之间的融合
*2015/3/26:  融合与ZVD发现通信异常，问题排查中---后排查更改CMD RAM分配即恢复正常
*2015/3/19: 改串口B为串口A
*2015/3/3: 添加三处校正频率ADC采样时钟 定时器
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/

#include "DSP28x_Project.h"
#include "Header.h"



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

/*=============================引用变量 Start=============================*/

extern Uint16 SampleDataSave[SAMPLE_LEN + 1]; //采样数据存储 转存 (SAMPLE_LEN + 1)长度  from SampProcess.c
extern float SampleDataSavefloat[SAMPLE_LEN + 1]; //Uint16 到 float 变换，后期可以考虑重用空间，减少ram使用  from SampProcess.c
extern struct FreqCollect FreqMonitor;            //监控频率 from MonitorCalculate.c

/*=============================引用变量 End=============================*/


int main(void)
{
	float samplePriod = 0;
	float freqArray[7] = { 0 };
	Uint8 freqLen = 0;

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
	PieVectTable.TINT0 = &cpu_timer0_isr;
	PieVectTable.ADCINT1 = &ADC_INT1_ISR;
	//PieVectTable.ADCINT2 = &ADC_INT2_Potect_ISR;
	//PieVectTable.SCIRXINTB = &ScibRX_ISR;
	//PieVectTable.SCITXINTB = &ScibTX_ISR;
	//PieVectTable.SCIRXINTA = &SciaRX_ISR;
	//PieVectTable.SCITXINTA = &SciaTX_ISR;
	PieVectTable.EPWM4_INT = &epwm4_timer_isr;
	EDIS;
	// This is needed to disable write to EALLOW protected registers

	//针对Flash模式，将关键算法程序从Flash拷贝到RAM中运行
#ifdef FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32) &RamfuncsLoadSize);
	InitFlash();
#endif

	//Step 4. Initialize the Device Peripheral.
	//EPwm4TimerInit();

	InitAdc(); //初始化ADC
	InitCpuTimers(); //初始化CPU寄存器
	ConfigCpuTimer(&CpuTimer0, 80, 1000000); //配置CPU在80M工作频率下，中断周期1000000us
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0 启动定时器

	//使能外PIE中断向量
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1; //TIMER0 	// Enable TINT0 in the PIE: Group 1 interrupt 7
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1; //SCIA -RX
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1; //SCIA -TX
	//PieCtrlRegs.PIEIER9.bit.INTx3 = 1; //SCIB -RX
	PieCtrlRegs.PIEIER9.bit.INTx4 = 1; //SCIB -TX

	PieCtrlRegs.PIEIER1.bit.INTx1 = 1; //ADCINT1
	//PieCtrlRegs.PIEIER1.bit.INTx2 = 1;//ADCINT2  Group 1 interrupt 2
	PieCtrlRegs.PIEIER3.bit.INTx4 = PWM4_INT_ENABLE;

	IER |= M_INT1;		//TIMER0 ADCINT1 ADCINT2
	IER |= M_INT3;        // Enable CPU INT3 which is connected to EPWM1-6 INT:
	EINT;
	// Enable Global interrupt INTM
	ERTM;
	// Enable Global realtime interrupt DBGM

	InitSampleProcessData(); //采样数据初始化
	InitMonitorCalData(); //监控数据计算初始化
	ConfigADC_Monitor(12500);  //ADC 采样初始化 设定采样周期 12500属于定时器计数长度

//  GenRTUFrame(0x01, 0x02, sendData, 16,SendFrameData, &len);
//  SendFrame(SendFrameData, len);
	//调试使用
	// StartSample();
	while (1) {
		//测频模块处理
		//理论上将是20ms一个循环
		if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_COMPLTE) //采样完成
		{
			CalFreq(SampleDataSavefloat);

			if (freqLen < 7) //计数长度 每7求取平均值计算周期
					{
				freqArray[freqLen++] = FreqMonitor.FreqReal;

			} else {
				freqLen = 0;
				FreqMonitor.FreqMean = MidMeanFilter(freqArray, 7); //
			}
			FreqMonitor.FreqReal = FreqMonitor.FreqMean;
			samplePriod = 15625.0f / FreqMonitor.FreqMean; //计算实时采样周期 1e6/64
			SetSamplePriod(samplePriod);
			StartSample(); //继续开始采样，再次测频
		}
	}

	

}




/*
          FS = (FreqReal * (float)N_FFT);
                 for (i = 0; i < 2*N_FFT; i++)
                 {

                   SimSinvalue[i] =0.1*exp(-20.0*(i+1000)/FS) + 1*cos(2.0*PI*i*f1/FS + setPhase) + 0.2*cos(2.0*PI*i*2*f1/FS + 1.0/6.0*PI)
                            + 0.2*cos(2.0*PI*i*3*f1/FS + 2.0/3.0*PI)+ 0.1*cos(2.0*PI*i*4*f1/FS + 1.0/3.0*PI)
                          + 0.1*cos(2.0*PI*i*5*f1/FS + 1.0/7.0*PI)+ 0.1*cos(2.0*PI*i*6*f1/FS + 1.0/7.0*PI)
                          + 0.1*cos(2.0*PI*i*7*f1/FS + 2.0/5.0*PI)+ 0.1*cos(2.0*PI*i*12*f1/FS + 1.0/14.0*PI)
                          ;

                   SimSinvalue[i] = (float)((int32)((SimSinvalue[i] + 2.5)*0.2*2048));

                 }
*/

