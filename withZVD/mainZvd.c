/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Main.c --- 此方法基于过零检测
*文件标识:
*创建日期： 2015年1月28日
*摘要:
*2015/3/25: 采样率6400， 连续采集10个周波 128*10
*2015/3/19 改串口B为串口A
*2015/3/3: 添加三处校正频率ADC采样时钟 定时器
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/


#include "DSP28x_Project.h"
#include "Header.h"
#include "SoftOverZero.h"



//#define FLASH   1
#ifdef FLASH
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;
#endif

struct frameRtu sendFrame, recvFrame; //通讯发送帧与接收帧

extern Uint8  volatile SendFrameData[SEND_FRAME_LEN];
extern Uint16 SampleDataSave[SAMPLE_LEN + 1];
extern float SampleDataSavefloat[SAMPLE_LEN + 1];
extern struct FreqStruct FreqMonitor; //监控频率



//测频算法区域
float FS = 6400; //采样率
float diffPhase = 0; //相位差
float SimSinvalue[2*N_FFT]; //余弦值  模拟值

struct_Base Xiangjiao; //相角
float Fq = 0;  //求得到的频率
float DiffFreq = 0; //求取频率与实际频率差值
float setPhase =  7.0/8.0* PI;
float f1 = 50;


float OutTest[10*SAMPLE_LEN_PRIOD];
extern const float COEEF_BP256[256];
float OverIndex[20] = {0};
float T0 = 0,T1 = 0, T2= 0, T3 = 0,T4 = 0,T5 = 0, T6= 0, T7 = 0;
float Tp[7] = {0};
Uint16 OverCn = 0;

extern const float COEEF_BP256[256];
int main(void)
{
  float maxP = 0, minP = 0, sumPriod = 0;
  Uint8 i = 0;
  //Uint8 sendData[16] = {0}, i = 0;
  //float samplePriod = 0;
 // float freqArray[7] = {0};
  //Uint8 freqLen = 0;
  //for (i = 0; i < 16; i++)
  //  {
  //    sendData[i] = i;
  //  }

  InitSysCtrl();

  InitDeviceIO();

  DINT;	// Step 3. Clear all interrupts and initialize PIE vector table:
  InitPieCtrl();
  IER = 0x0000;
  IFR = 0x0000;
  InitPieVectTable();
  EALLOW;  // This is needed to write to EALLOW protected registers
  PieVectTable.TINT0 = &cpu_timer0_isr;
  PieVectTable.ADCINT1 = &ADC_INT1_ISR;
  //PieVectTable.ADCINT2 = &ADC_INT2_Potect_ISR;
  PieVectTable.SCIRXINTA = &SciaRX_ISR;
  PieVectTable.SCITXINTA = &SciaTX_ISR;

  PieVectTable.EPWM4_INT = &epwm4_timer_isr;

  EDIS;    // This is needed to disable write to EALLOW protected registers

#ifdef FLASH
  memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize);
  InitFlash();
#endif

	 //Step 4. Initialize the Device Peripheral.
  //EPwm4TimerInit();
  InitSciaGpio(); //配置GPIO12 GPIO28 作为SCIA输入输出
	 //InitSciAny(&ScibRegs, 9600);
  SciAnyFifoInit(&SciaRegs);
	//InitScib(); //
  InitAdc();
  InitCpuTimers();
 // ConfigCpuTimer(&CpuTimer0, 80, 1000000);
 // CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

  PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
  PieCtrlRegs.PIEIER1.bit.INTx7 = 1;//TIMER0 	// Enable TINT0 in the PIE: Group 1 interrupt 7
  PieCtrlRegs.PIEIER9.bit.INTx1 = 1; //SCIA -RX
  PieCtrlRegs.PIEIER9.bit.INTx2 = 1; //SCIA -TX
  PieCtrlRegs.PIEIER1.bit.INTx1 = 1;//ADCINT1
	//PieCtrlRegs.PIEIER1.bit.INTx2 = 1;//ADCINT2  Group 1 interrupt 2
  PieCtrlRegs.PIEIER3.bit.INTx4 = PWM4_INT_ENABLE;

  IER |= M_INT1;		//TIMER0 ADCINT1 ADCINT2
  IER |= M_INT9;  //SCITXB SCIRXB SCITXA SCIRXA
  IER |= M_INT3;   // Enable CPU INT3 which is connected to EPWM1-6 INT:
  EINT;   // Enable Global interrupt INTM
  ERTM;   // Enable Global realtime interrupt DBGM

  //通讯数据初始化
  ReciveFrameDataInit();
  sendFrame.address =  LOCAL_ADDRESS;
  //采样数据初始化
  InitSampleProcessData();
  //监控数据计算初始化
  InitMonitorCalData();
  //ADC 采样初始化
  ConfigADC_Monitor(6250); //20*10^3/64*80/2  每周期64点

//  GenRTUFrame(0x01, 0x02, sendData, 16,SendFrameData, &len);
//  SendFrame(SendFrameData, len);
  //调试使用
 // StartSample();
  while(1)
     {
       //测频模块处理
	  if (SampleDataSavefloat[SAMPLE_LEN] == SAMPLE_COMPLTE) //采样完成
		  {
			  FIR_Self(COEEF_BP256, 256,SampleDataSavefloat, 10*SAMPLE_LEN_PRIOD,OutTest);
			  OverCn = SearchZero(OutTest, 0, 1000, 0, OverIndex);
			 // T0 = 1e6 / f1;
			  Tp[0] = (OverIndex[OverCn-1] -  OverIndex[OverCn-3]) ;//* 156.25  312.5000f;  // 1e6/Fs;
			  Tp[1] = (OverIndex[OverCn-2] -  OverIndex[OverCn-4]) ;//* 156.25   312.5000f;  // 1e6/Fs;
			  Tp[2] = (OverIndex[OverCn-3] -  OverIndex[OverCn-5]) ;
			  Tp[3] = (OverIndex[OverCn-4] -  OverIndex[OverCn-6]) ;
			  Tp[4] = (OverIndex[OverCn-5] -  OverIndex[OverCn-7]) ;
			  Tp[5] = (OverIndex[OverCn-6] -  OverIndex[OverCn-8]) ;
			  Tp[6] = (OverIndex[OverCn-7] -  OverIndex[OverCn-9]) ;
			  minP = Tp[0];
			  maxP = Tp[0];
			  sumPriod = 0;
			  for (i = 0 ; i < 7; i++)
			  {
				  if (minP > Tp[i])
				  {
					  minP = Tp[i];
				  }
				  if (maxP < Tp[i])
				  {
				  	  maxP = Tp[i];
				  }
				  sumPriod += Tp[i];
			  }
			  sumPriod = sumPriod - minP - maxP;
			  T3 = sumPriod * 31.2500f;//156.25 /7
			  FreqMonitor.FreqReal = 1e6/ T3;
			  FreqMonitor.FreqMean =  FreqMonitor.FreqReal;
			  SampleDataSavefloat[SAMPLE_LEN] = SAMPLE_NOT_FULL;
			  StartSample();
		  }
       //通讯处理
       ReciveBufferDataDealing(&sendFrame, &recvFrame);
       if (recvFrame.completeFlag == TRUE)
         {
           ExecuteFunctioncode(&recvFrame);
         }
     }
  while(1);
	

}






