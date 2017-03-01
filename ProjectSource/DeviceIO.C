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

   //LED1-3
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO29= 1;
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO28= 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO9= 1;

    //OUT1-8
	GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO5= 1;
//	GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;//禁止上拉

	GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO4= 1;

	GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO21= 1;

	GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO20= 1;

	GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO23= 1;

	GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO22= 1;

	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO24= 1;

	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO14= 1;

    //SCLA SCDA
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;

	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;//默认启动内部上拉
	GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 2;//质量控制



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
	OFF_LED2;
	ON_LED3;
}

