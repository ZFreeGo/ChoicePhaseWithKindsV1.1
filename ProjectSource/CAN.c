/*
 * CAN.c
 *
 *  Created on: 2017年4月1日
 *      Author: ZYF
 */

#include "CAN.h"
#include "HEADER.h"
#include "stdtype.h"
#include "DSP28x_Project.h"
#include "DeviceNet.h"
#include "BasicModule.h"
#include "RefParameter.h"



uint16_t  InitStandardCAN(uint16_t id, uint16_t mask)
{

    struct ECAN_REGS ECanaShadow;

	InitECanGpio();
	InitECana(); // Initialize eCAN-A module

    // Mailboxs can be written to 16-bits or 32-bits at a time
    // Write to the MSGID field of TRANSMIT mailboxes MBOX0 - 15
	//IDE[31]= 0, AME[30] =0, P1085
    //ID[28:0] --In standard identifier mode, if the IDE bit (MSGID.31) = 0, the message identifier is stored in bits
    //ID.28:18. In this case, bits ID.17:0 have no meaning.
	//In extended identifier mode, if the IDE bit (MSGID.31) = 1, the message identifier is stored in bits
	//ID.28:0.
    //ECanaMboxes.MBOX0.MSGID.all =  0x15500000;

    // Write to the MSGID field of RECEIVE mailboxes MBOX16 - 31
    ECanaMboxes.MBOX16.MSGID.all = 0x40840000;//28:18   ID:0x21  AME=1

    // Configure Mailboxes 0-15 as Tx, 16-31 as Rx
    // Since this write is to the entire register (instead of a bit
    // field) a shadow register is not required.
    ECanaRegs.CANMD.all =  0x00010000;


    ECanaRegs.CANOPC.all = 0x00010000;
    // Enable all Mailboxes */
    // Since this write is to the entire register (instead of a bit
    // field) a shadow register is not required.
    ECanaRegs.CANME.all = 0x00010001;

    // Specify that 8 bits will be sent/received
    // ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;


    // Write to the mailbox RAM field of MBOX0 - 15
    // ECanaMboxes.MBOX0.MDL.all = 0x9555AAA0;
    // ECanaMboxes.MBOX0.MDH.all = 0x9555AAA0;



    ECanaLAMRegs.LAM16.all = 0xFFFFFFFF; //屏蔽

    // Since this write is to the entire register (instead of a bit
    // field) a shadow register is not required.
    EALLOW;
    ECanaRegs.CANMIM.all = 0x00010000;//MAILBOX 16  使能接收
    ECanaRegs.CANGIM.bit.I0EN = 1;//中断0使能
    ECanaRegs.CANGIM.bit.EPIM = 1;//使能错误中断Error-passive interrupt mask
    ECanaRegs.CANGIM.bit.BOIM = 1;//Bus-off interrupt mask
    ECanaRegs.CANGIM.bit.RMLIM = 1;//Received-message-lost interrupt mask

    // Configure the eCAN for self test mode
    // Enable the enhanced features of the eCAN.



    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.DBO = 1;//采用小端模式
    ECanaShadow.CANMC.bit.STM = 0;    //0-Normal模式   1- Configure CAN for self-test mode
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;


    EDIS;

    return 0xFF;

}

/********************************************
*函数名：CANOneSendByTX0()
*形参：: uint16* pID  11bitID标识, uint8 * pbuff 缓冲数据, uint8 len 数据长度
*返回值：uint8 —— 发送数据总长度  0xFF--数据出错
*功能： 通过TX0发送带有CRC16的帧数据
**********************************************/
 uint8_t CANSendData(uint16_t id, uint8_t * pbuff, uint8_t len)
 {
	 if (len > 0)
	 {
		 uint8_t sendData[8] = { 0 };
		 uint8_t remainLen = len % 8; //余数
		 uint8_t loop = len / 8;//循环次数
		 uint8_t  i = 0;

		 uint32_t data1 = 0;
		 uint32_t data2 = 0;
		 uint32_t data3 = 0;
		 uint32_t sendCount= 0 ;//发送计数

		 ECanaRegs.CANME.all = 0x00010000;
		 ECanaMboxes.MBOX0.MSGID.bit.IDE = 0; //标准帧 When AMI = 0:
		 ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = id;
		 ECanaRegs.CANME.all = 0x00010001;

		 for (i = 0; i < loop; i++)//8的整数倍
		 {
			 data1 = pbuff[8 * i + 1];
			 data2 = pbuff[8 * i + 2];
			 data3 = pbuff[8 * i + 3];
			 ECanaMboxes.MBOX0.MDL.all = (data3 << 24) | (data2 << 16) | (data1 << 8) | pbuff[8 * i + 0];

			 data1 = pbuff[8 * i + 5];
			 data2 = pbuff[8 * i + 6];
			 data3 = pbuff[8 * i + 7];
			 ECanaMboxes.MBOX0.MDH.all = (data3 << 24) | (data2 << 16) | (data1 << 8) | pbuff[8 * i + 4];

			 ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
			 ECanaRegs.CANTRS.all = 0x00000001;  // Set TRS for all transmit mailboxes
			 while (ECanaRegs.CANTA.all != 0x00000001)
			 {
				if( g_CANErrorStatus != 0)
				{
					return 0xFF;
				}
				//错误超限
				if(sendCount++ > 10000)
				{
					return 0xFF;
				}

			 }  // Wait for all TAn bits to be set..
			 ECanaRegs.CANTA.all = 0x000000001;   // Clear all TAn
		 }
		 if ( remainLen > 0)
		 {
			 memcpy(sendData, pbuff + 8 * loop,  remainLen);

			 data1 = sendData[1];
			 data2 = sendData[2];
			 data3 = sendData[3];
			 ECanaMboxes.MBOX0.MDL.all = (data3 << 24) | (data2 << 16) | (data1 << 8) | sendData[0];

			 data1 = sendData[5];
			 data2 = sendData[6];
			 data3 = sendData[7];
			 ECanaMboxes.MBOX0.MDH.all = (data3 << 24) | (data2 << 16) | (data1 << 8) | sendData[4];



			 ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = remainLen;
			 ECanaRegs.CANTRS.all = 0x00000001;  // Set TRS for all transmit mailboxes
			 while (ECanaRegs.CANTA.all != 0x00000001)// Wait for all TAn bits to be set..
			 {
				 if( g_CANErrorStatus != 0)
				 {
	 				return 0xFF;
	 				//TODO:超时报警
	 			 }
				 //错误超限
				if(sendCount++ > 10000)
				{
					return 0xFF;
				}
			 }
			 ECanaRegs.CANTA.all = 0x000000001;   // Clear all TAn
		 }
		 return len;
	 }
	 return 0xFF;



 }





//uint32_t  TestMbox1 = 0;
//uint32_t  TestMbox2 = 0;
uint32_t dataError = 0;
uint8_t ReceiveData[10] = { 0 }; //应该注意被意外覆盖
__interrupt void Can0Recive_ISR(void)
{

	uint16_t id = 0, k = 0;
	uint8_t len = 0;


	if(( ECanaRegs.CANRMP.all & 0x00010000) == 0x00010000)//是否为Box16
	{
       // TestMbox1 = ECanaMboxes.MBOX16.MDL.all;
        //TestMbox2 = ECanaMboxes.MBOX16.MDH.all;
    	ECanaRegs.CANRMP.all = 0x00010000;

    	dataError = ECanaRegs.CANES.all;
    	len = ECanaMboxes.MBOX16.MSGCTRL.bit.DLC;
    	id = ECanaMboxes.MBOX16.MSGID.bit.STDMSGID;

    	ReceiveData[0] = GET_BIT0_7( ECanaMboxes.MBOX16.MDL.all);
    	ReceiveData[1] = GET_BIT8_15( ECanaMboxes.MBOX16.MDL.all);
    	ReceiveData[2] = GET_BIT16_23( ECanaMboxes.MBOX16.MDL.all);
    	ReceiveData[3] = GET_BIT24_31( ECanaMboxes.MBOX16.MDL.all);
    	ReceiveData[4] = GET_BIT0_7( ECanaMboxes.MBOX16.MDH.all);
    	ReceiveData[5] = GET_BIT8_15( ECanaMboxes.MBOX16.MDH.all);
    	ReceiveData[6] = GET_BIT16_23( ECanaMboxes.MBOX16.MDH.all);
    	ReceiveData[7] = GET_BIT24_31( ECanaMboxes.MBOX16.MDH.all);

    	DeviceNetReciveCenter(&id, ReceiveData, len);
    	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    	return;

	}
	//ECanaRegs.CANGIM.bit.EPIM = 1;//使能错误中断Error-passive interrupt mask
	//    ECanaRegs.CANGIM.bit.BOIM = 1;//Bus-off interrupt mask
	//    ECanaRegs.CANGIM.bit.RMLIM = 1;//Received-message-lost interrupt mask
	if (ECanaRegs.CANGIF0.bit.BOIF0)//Bus off interrupt flag
	{
		 k = 0;
		while(k++ < 100)
		{
			ON_LED3;
			DelayMs(100);
		}
		ECanaRegs.CANGIF0.bit.BOIF0 = 1;
	}
	if (ECanaRegs.CANGIF0.bit.RMLIF0)//Received-message-lost interrupt flag
	{
		 k = 0;
		while(k++ < 100)
		{
			ON_LED3;
			DelayMs(100);
		}
		ECanaRegs.CANGIF0.bit.RMLIF0 = 1;
	}
	if (ECanaRegs.CANGIF0.bit.EPIF0)//使能错误中断Error-passive interrupt flag
	{
		g_CANErrorStatus = ECanaRegs.CANES.all;
		 k = 0;
		while(k++ < 100)
		{
			ON_LED3;
			DelayMs(100);
		}
		ECanaRegs.CANGIF0.bit.EPIF0 = 1;
	}
   // Acknowledge this interrupt to receive more interrupts from group 9
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}
