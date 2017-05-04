/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:ConfigADC.c
*�ļ���ʶ:
*�������ڣ� 2015��1��31��
*ժҪ:
*2015/1/31: ADC�������õȵ�
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#include"ConfigADC.h"
#include"DSP28x_Project.h"
#include "Header.h"


/*=============================ȫ�ֱ������� Start=============================*/
/*=============================ȫ�ֱ������� End=============================*/

/*=============================���ñ��� extern Start=============================*/
extern Uint16 SampleIndex; //�������� from SampleProcess.c
/*=============================���ñ��� extern End=============================*/


/********************************************************************
 * ��������ConfigADC_Monitor()
 * ������NULL
 * ����ֵ��NULL
 * ���ܣ����ü���������� ÿ�ܲ�64��  ADCINA0 ��ѹ����
 *���ú����� null
 *�����ⲿ������ null
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
    //��������
    EPwm1Regs.ETSEL.bit.SOCAEN	= 1;		// Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL	= 4;		// Select SOC from CMPA on upcount
    EPwm1Regs.ETPS.bit.SOCAPRD 	= 1;		// Generate pulse on 1st event
    EPwm1Regs.CMPA.half.CMPA 	= 0;	// 20*10^3/64*80/2 = 12500  default /2
    EPwm1Regs.TBPRD 			= priod;	// 12500  * FREQ_CALI_RATE// Set period for ePWM1   (TBPRD-CMPA)*2 ���
    //EPwm1Regs.TBCTL.bit.CTRMODE = 0;		// count up and start
}

/********************************************************************
 * ��������StartSample()
 * ������void
 * ����ֵ��void
 * ���ܣ���ʼ��������������������
 *���ú����� null
 *�����ⲿ������ null
 ********************************************************************/
void StartSample(void)
{
	SampleIndex = 0;
	EPwm1Regs.TBCTL.bit.CTRMODE = 0;            // count up and start
}

/********************************************************************
 * ��������StopSample()
 * ������NULL
 * ����ֵ��NULL
 * ���ܣ�ֹͣ����
 * ���ú����� null
 * �����ⲿ������ null
 ********************************************************************/
void StopSample(void)
{
	EPwm1Regs.TBCTL.bit.CTRMODE = 0b11;            // count up and start
}

/********************************************************************
 * ��������SetSamplePriod()
 * ������NULL
 * ����ֵ��NULL
 * ���ܣ����ò�������,��λus  ϵͳָ��ʱ��80M us<1638us
 * ���ú����� null
 * �����ⲿ������ null
 ********************************************************************/
void SetSamplePriod(float us)
{
  //ֹͣ��ʱ
  EPwm1Regs.TBCTL.bit.CTRMODE = 0b11;
  //��������
  EPwm1Regs.ETSEL.bit.SOCAEN  = 1;            // Enable SOC on A group
  EPwm1Regs.ETSEL.bit.SOCASEL = 4;            // Select SOC from CMPA on upcount
  EPwm1Regs.ETPS.bit.SOCAPRD  = 1;            // Generate pulse on 1st event
  EPwm1Regs.CMPA.half.CMPA    = 0;    // 20*10^3/64*80/2 = 12500  default /2
  EPwm1Regs.TBPRD                     = (Uint16)(us * 40.0 * OVER_SAMPLE_RATE * FREQ_CALI_RATE);        // Set period for ePWM1   (TBPRD-CMPA)*2 ���
 // EPwm1Regs.TBCTL.bit.CTRMODE = 0;            // count up and start

}


