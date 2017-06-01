/*
 * ECAP.c
 *
 *  Created on: 2017年6月1日
 *      Author: ZYF
 */

#include "ECAP.h"
#include "RefParameter.h"
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "stdType.h"
#include "FrequrncyCaluate.h"

static float CapFreqArray[7] = { 0 };
static uint8_t CapFreqIndex = 0;

static void InitECapture2(void);


void ConfigECapFrquency(void)
{
     InitECap2Gpio();
     InitECapture2();

     CapFreqIndex = 0;

}

static void InitECapture2(void)
{
   ECap2Regs.ECEINT.all = 0x0000;             // Disable all capture interrupts
   ECap2Regs.ECCLR.all = 0xFFFF;              // Clear all CAP interrupt flags
   ECap2Regs.ECCTL1.bit.CAPLDEN = 0;          // Disable CAP1-CAP4 register loads
   ECap2Regs.ECCTL2.bit.TSCTRSTOP = 0;        // Make sure the counter is stopped

   // Configure peripheral registers
   ECap2Regs.ECCTL2.bit.CONT_ONESHT = 1;      // One-shot
   ECap2Regs.ECCTL2.bit.STOP_WRAP = 3;        // Stop at 4 events
   ECap2Regs.ECCTL1.bit.CAP1POL = 1;          // Falling edge
   ECap2Regs.ECCTL1.bit.CAP2POL = 0;          // Rising edge
   ECap2Regs.ECCTL1.bit.CAP3POL = 1;          // Falling edge
   ECap2Regs.ECCTL1.bit.CAP4POL = 0;          // Rising edge
   ECap2Regs.ECCTL1.bit.CTRRST1 = 1;          // Difference operation 复位计数器
   ECap2Regs.ECCTL1.bit.CTRRST2 = 1;          // Difference operation 复位计数器
   ECap2Regs.ECCTL1.bit.CTRRST3 = 1;          // Difference operation 复位计数器
   ECap2Regs.ECCTL1.bit.CTRRST4 = 1;          // Difference operation 复位计数器
   ECap2Regs.ECCTL2.bit.SYNCI_EN = 1;         // Enable sync in
   ECap2Regs.ECCTL2.bit.SYNCO_SEL = 0;        // Pass through
   ECap2Regs.ECCTL1.bit.CAPLDEN = 1;          // Enable capture units

   ECap2Regs.ECCTL2.bit.TSCTRSTOP = 1;        // Start Counter
   ECap2Regs.ECCTL2.bit.REARM = 1;            // arm one-shot
   ECap2Regs.ECCTL1.bit.CAPLDEN = 1;          // Enable CAP1-CAP4 register loads
   ECap2Regs.ECEINT.bit.CEVT4 = 1;            // 4 events = interrupt

}

__interrupt void ecap2_isr(void)
{

   // Cap input is syc'ed to SYSCLKOUT so there may be
   // a +/- 1 cycle variation


	g_SystemVoltageParameter.perodCap = (ECap2Regs.CAP2 +
			ECap2Regs.CAP4) * 0.0125;

	if (CapFreqIndex < 7) //计数长度 每7求取平均值计算周期
	{
		CapFreqArray[CapFreqIndex++] = g_SystemVoltageParameter.perodCap;

	}
	else
	{
		CapFreqIndex = 0;
		g_SystemVoltageParameter.perodMeanCap =  MidMeanFilter(CapFreqArray, 7); //计算平均值
	}


   ECap2Regs.ECCLR.bit.CEVT4 = 1;
   ECap2Regs.ECCLR.bit.INT = 1;
   ECap2Regs.ECCTL2.bit.REARM = 1;

   // Acknowledge this interrupt to receive more interrupts from group 4
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}
