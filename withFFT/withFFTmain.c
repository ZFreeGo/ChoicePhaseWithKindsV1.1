/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:Main.c --- �˷�������FFT
*�ļ���ʶ:
*�������ڣ� 2015��1��28��
*ժҪ:
*�� ����������˵���ĵ�.txt
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
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

/*=============================ȫ�ֱ������� Start=============================*/


//��Ƶ�㷨����
//float FS = 3200; //������
/*=============================ȫ�ֱ������� End=============================*/




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

	//���Flashģʽ�����ؼ��㷨�����Flash������RAM������
#ifdef FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32) &RamfuncsLoadSize);
	InitFlash();
#endif

	//Step 4. Initialize the Device Peripheral.


	InitStandardCAN(0, 0);
	InitAdc(); //��ʼ��ADC
	InitCpuTimers(); //��ʼ��CPU�Ĵ���
	ConfigCpuTimer(&CpuTimer0, 80, 1000); //����CPU��80M����Ƶ���£��ж�����1000us
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0 ������ʱ��

	//ʹ����PIE�ж�����
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

	InitSampleProcessData(); //�������ݳ�ʼ��
	InitMonitorCalData(); //������ݼ����ʼ��
	ConfigADC_Monitor(12500);  //ADC ������ʼ�� �趨�������� 12500���ڶ�ʱ���������ȣ�ÿ�ܲ�64��
	ConfigECapFrquency();
	ActionInit();
	InitDeviceNet();

	//InitEPwmTimer();
	//����ʹ��
	StartSample();//������������



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


