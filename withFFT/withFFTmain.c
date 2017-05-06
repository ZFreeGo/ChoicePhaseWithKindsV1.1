/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:Main.c --- �˷�������FFT
*�ļ���ʶ:
*�������ڣ� 2015��1��28��
*ժҪ:
*2017/5/4����Ӧ�°汾Ӳ����ͬ����������Ӳ���汾V1.1���������ΪV1.2
*2016/11/9:
*
*2016/11/9: Դ�ļ����ƣ��İ汾ΪV1.1 ������Ӧ���޸ġ�
*           ����C����Լ���������������޸ģ��޸� frameRtu ΪFrameRtu
*           ɾ������B����,
*
*
*
*2015/6/29:��Ӻ�բ������ã�������բ�Ƕȣ���բʱ��Ȳ�����
*2015/6/18:��ӽ��ճ�ʱ������ӵȴ�ͬ���������ֳ�ʱ�����Ӱ�������ʱ���������ȡ�����Ժ�Ӧ�ü��ϣ�������档
*�޸�lib�ļ��ο���ʹ���ܹ���Ӧ�°汾��ͨ��FLASH��̵�memcpy���С�
*2015/6/15: ���485ͨѶ���������·֮����ں�
*2015/3/26:  �ں���ZVD����ͨ���쳣�������Ų���---���Ų����CMD RAM���伴�ָ�����
*2015/3/19: �Ĵ���BΪ����A
*2015/3/3: �������У��Ƶ��ADC����ʱ�� ��ʱ��
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
	//InitECanGpio();
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
	PieVectTable.EPWM4_INT = &epwm4_timer_isr;
	PieVectTable.ECAN0INTA = &Can0Recive_ISR;

	EDIS;
	// This is needed to disable write to EALLOW protected registers

	//���Flashģʽ�����ؼ��㷨�����Flash������RAM������
#ifdef FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32) &RamfuncsLoadSize);
	InitFlash();
#endif

	//Step 4. Initialize the Device Peripheral.
	//EPwm4TimerInit();


	//InitECana(); // Initialize eCAN-A module

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

	IER |= M_INT1;		//TIMER0 ADCINT1 ADCINT2
	IER |= M_INT3;        // Enable CPU INT3 which is connected to EPWM1-6 INT:
	IER |= M_INT9;

	EINT;
	// Enable Global interrupt INTM
	ERTM;
	// Enable Global realtime interrupt DBGM

	InitSampleProcessData(); //�������ݳ�ʼ��
	InitMonitorCalData(); //������ݼ����ʼ��
	ConfigADC_Monitor(12500);  //ADC ������ʼ�� �趨�������� 12500���ڶ�ʱ���������ȣ�ÿ�ܲ�64��

	InitDeviceNet();
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

