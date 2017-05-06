/***********************************************
*Copyright(c) 2016,FreeGo
*保留所有权利
*文件名称:DeviceIo.c
*文件标识:
*创建日期： 2016年11月9日
*摘要:
*初始化基本端口函数，基本端口宏定义
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#ifndef __CONFIGGPIO_H_
#define __CONFIGGPIO_H_

//取反操作
#define  TOGGLE_LED1  {GpioDataRegs.GPATOGGLE.bit.GPIO6 = 1;}
#define  TOGGLE_LED2  {GpioDataRegs.GPATOGGLE.bit.GPIO16 = 1; }
#define  TOGGLE_LED3  {GpioDataRegs.GPATOGGLE.bit.GPIO7 = 1; }

//熄灭
#define  OFF_LED1  {GpioDataRegs.GPASET.bit.GPIO6 = 1;}
#define  OFF_LED2  {GpioDataRegs.GPASET.bit.GPIO16 = 1;  }
#define  OFF_LED3  {GpioDataRegs.GPASET.bit.GPIO7 = 1; }

//点亮操作
#define  ON_LED1  {GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;}
#define  ON_LED2  {GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;}
#define  ON_LED3  {GpioDataRegs.GPACLEAR.bit.GPIO7 = 1; }


//永磁控制 同步端口控制
#define SET_OUTA1_H {GpioDataRegs.GPASET.bit.GPIO12 = 1;}
#define SET_OUTA1_L {GpioDataRegs.GPACLEAR.bit.GPIO12 = 1;}

#define GET_INA1  GpioDataRegs.GPADAT.bit.GPIO9


#define SET_OUTA2_H {GpioDataRegs.GPASET.bit.GPIO39 = 1;}
#define SET_OUTA2_L {GpioDataRegs.GPACLEAR.bit.GPIO39 = 1;}

#define GET_INA2  GpioDataRegs.GPADAT.bit.GPIO19

#define SET_OUTB1_H {GpioDataRegs.GPASET.bit.GPIO4 = 1;}
#define SET_OUTB1_L {GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;}

#define GET_INB1  GpioDataRegs.GPADAT.bit.GPIO5

#define SET_OUTB2_H {GpioDataRegs.GPASET.bit.GPIO21 = 1;}
#define SET_OUTB2_L {GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;}

#define GET_INB2  GpioDataRegs.GPADAT.bit.GPIO15

#define SET_OUTB3_H {GpioDataRegs.GPASET.bit.GPIO20 = 1;}
#define SET_OUTB3_L {GpioDataRegs.GPACLEAR.bit.GPIO20 = 1;}

#define GET_INB3  GpioDataRegs.GPADAT.bit.GPIO26

#define SET_OUTB4_H {GpioDataRegs.GPASET.bit.GPIO23 = 1;}
#define SET_OUTB4_L {GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;}

#define GET_INB4  GpioDataRegs.GPADAT.bit.GPIO27


#define SET_SCLA_H {GpioDataRegs.GPBSET.bit.GPIO32 = 1;}
#define SET_SCLA_L  {GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;}
#define SET_SDAA_H {GpioDataRegs.GPBSET.bit.GPIO33 = 1;}
#define SET_SDAA_L  {GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;}

#define SDAA_DIR_IN {GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;}
#define SDAA_DIR_OUT {GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1;}

#ifdef	__cplusplus
extern "C" {
#endif

void InitDeviceIO(void);
Uint16 GetKeyValue(void);


#ifdef	__cplusplus
}
#endif


#endif
