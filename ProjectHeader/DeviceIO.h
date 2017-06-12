/***********************************************
*Copyright(c) 2016,FreeGo
*��������Ȩ��
*�ļ�����:DeviceIo.c
*�ļ���ʶ:
*�������ڣ� 2016��11��9��
*ժҪ:
*��ʼ�������˿ں����������˿ں궨��
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
************************************************************/
#ifndef __CONFIGGPIO_H_
#define __CONFIGGPIO_H_

//ȡ������
#define  TOGGLE_LED1  {GpioDataRegs.GPATOGGLE.bit.GPIO9 = 1;}
#define  TOGGLE_LED2  {GpioDataRegs.GPATOGGLE.bit.GPIO29 = 1; }
#define  TOGGLE_LED3  {GpioDataRegs.GPATOGGLE.bit.GPIO12 = 1; }
#define  TOGGLE_LED4  {GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1; }
#define  TOGGLE_LED5  {GpioDataRegs.GPATOGGLE.bit.GPIO28 = 1; }

//Ϩ��
#define  OFF_LED1  {GpioDataRegs.GPASET.bit.GPIO9 = 1;}
#define  OFF_LED2  {GpioDataRegs.GPASET.bit.GPIO29 = 1;  }
#define  OFF_LED3  {GpioDataRegs.GPASET.bit.GPIO12 = 1; }
#define  OFF_LED4  {GpioDataRegs.GPASET.bit.GPIO18 = 1;  }
#define  OFF_LED5  {GpioDataRegs.GPASET.bit.GPIO28 = 1; }

//��������
#define  ON_LED1  {GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;}
#define  ON_LED2  {GpioDataRegs.GPACLEAR.bit.GPIO29 = 1;}
#define  ON_LED3  {GpioDataRegs.GPACLEAR.bit.GPIO12 = 1; }
#define  ON_LED4  {GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;}
#define  ON_LED5  {GpioDataRegs.GPACLEAR.bit.GPIO28 = 1; }

//���ſ��� ͬ���˿ڿ���
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

extern void InitDeviceIO(void);
extern void EnableatchDog(void);


#ifdef	__cplusplus
}
#endif


#endif
