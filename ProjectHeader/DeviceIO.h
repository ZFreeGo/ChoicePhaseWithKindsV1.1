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
#define  TOGGLE_LED1  {GpioDataRegs.GPATOGGLE.bit.GPIO9 = 1;}
#define  TOGGLE_LED2  {GpioDataRegs.GPATOGGLE.bit.GPIO28 = 1; } //周期信号
#define  TOGGLE_LED3  {GpioDataRegs.GPATOGGLE.bit.GPIO29 = 1; }//触发信号

//熄灭
#define  OFF_LED1  {GpioDataRegs.GPASET.bit.GPIO9 = 1;}
#define  OFF_LED2  {GpioDataRegs.GPASET.bit.GPIO28 = 1;  }
#define  OFF_LED3  {GpioDataRegs.GPASET.bit.GPIO29 = 1; }

//点亮操作
#define  ON_LED1  {GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;}
#define  ON_LED2  {GpioDataRegs.GPACLEAR.bit.GPIO28 = 1;}
#define  ON_LED3  {GpioDataRegs.GPACLEAR.bit.GPIO29 = 1; }


//永磁控制 同步端口控制
#define SET_OUTA1_H {GpioDataRegs.GPASET.bit.GPIO5 = 1;}
#define SET_OUTA1_L {GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;}
#define SET_OUTA2_H {GpioDataRegs.GPASET.bit.GPIO4 = 1;}
#define SET_OUTA2_L {GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;}
#define SET_OUTA3_H {GpioDataRegs.GPASET.bit.GPIO21 = 1;}
#define SET_OUTA3_L {GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;}
#define SET_OUTA4_H {GpioDataRegs.GPASET.bit.GPIO20 = 1;}
#define SET_OUTA4_L {GpioDataRegs.GPACLEAR.bit.GPIO20 = 1;}
#define SET_OUTA5_H {GpioDataRegs.GPASET.bit.GPIO23 = 1;}
#define SET_OUTA5_L {GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;}
#define SET_OUTA6_H {GpioDataRegs.GPASET.bit.GPIO22 = 1;}
#define SET_OUTA6_L {GpioDataRegs.GPACLEAR.bit.GPIO22 = 1;}
#define SET_OUTA7_H {GpioDataRegs.GPASET.bit.GPIO24 = 1;}
#define SET_OUTA7_L {GpioDataRegs.GPACLEAR.bit.GPIO24 = 1;}
#define SET_OUTA8_H {GpioDataRegs.GPASET.bit.GPIO14 = 1;}
#define SET_OUTA8_L {GpioDataRegs.GPACLEAR.bit.GPIO14 = 1;}


#ifdef	__cplusplus
extern "C" {
#endif

void InitDeviceIO(void);
Uint16 GetKeyValue(void);


#ifdef	__cplusplus
}
#endif


#endif
