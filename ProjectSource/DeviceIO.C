/***********************************************
*Copyright(c) 2016,FreeGo
*保留所有权利
*文件名称:DeviceIo.c
*文件标识:
*创建日期： 2016年11月9日
*摘要:
*初始化基本端口，按键,LED指示灯
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/

#include "DSP28x_Project.h"
#include "DeviceIO.h"


/**************************************************
 *函数名：InitDeviceIO ()
 *功能： 初始化外围LED灯
 *形参：void
 *返回值：void
 *调用函数： null
 *引用外部变量：null
****************************************************/
void InitDeviceIO(void)
{
    EALLOW;

   //LED1
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO6= 1;
    //LED2
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO16= 1;
    //LED3
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7= 1;

    //TXD1-LASER RXD1-LASER
    // A_TXDL11 A_RXDL11
    // A_TXDL21 A_RXDL21
    // A_TXDL31 A_RXDL31

    //TXD1-LASER
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO12= 1;
//	GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;//禁止上拉

	// RXD1-LASER
	GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO9= 0;


    //TXD2-LASER
	GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO39= 1;
//	GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;//禁止上拉

	// RXD2-LASER
	GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO19= 0;

	 // A_TXDL11
	GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO4= 1;

	// A_RXDL11
	GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO5= 0;

	 // A_TXDL21
	GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO21= 1;
	// A_RXDL21
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO15= 0;

	 // A_TXDL31
	GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO20= 1;
	 // A_RXDL31
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO26= 0;

	 // A_TXDL41
	GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO23= 1;
	 // A_RXDL41
	GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO27= 0;

    //SCLA SCDA
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;

	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;//默认启动内部上拉
	GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 2;//质量控制


    //C_SCLA C_SCDA
	GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;

	GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;//默认启动内部上拉
	GpioCtrlRegs.GPADIR.bit.GPIO2 = 0;
	GpioCtrlRegs.GPAQSEL1.bit.GPIO2 = 2;//质量控制

	//C_INT
	GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;//默认启动内部上拉
	GpioCtrlRegs.GPADIR.bit.GPIO3 = 0;
	GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 2;//质量控制

/******************************************************************************
	GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 2;//质量控制  6*Sample=6* 0x80*2*t
	GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 0x80;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0x80;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0x80;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0x80;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = 0x80;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD1 = 0x80;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD2 = 0x80;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD3 = 0x80;
*****************************************************************************/
	EDIS;

	ON_LED1;
	ON_LED2;
	ON_LED3;
	OFF_LED1;
	OFF_LED2;
	OFF_LED3;
}

