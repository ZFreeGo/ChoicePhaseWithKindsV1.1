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
#define  TOGGLE_LED2  {GpioDataRegs.GPATOGGLE.bit.GPIO28 = 1; } //�����ź�
#define  TOGGLE_LED3  {GpioDataRegs.GPATOGGLE.bit.GPIO29 = 1; }//�����ź�

//Ϩ��
#define  OFF_LED1  {GpioDataRegs.GPASET.bit.GPIO9 = 1;}
#define  OFF_LED2  {GpioDataRegs.GPASET.bit.GPIO28 = 1;  }
#define  OFF_LED3  {GpioDataRegs.GPASET.bit.GPIO29 = 1; }

//��������
#define  ON_LED1  {GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;}
#define  ON_LED2  {GpioDataRegs.GPACLEAR.bit.GPIO28 = 1;}
#define  ON_LED3  {GpioDataRegs.GPACLEAR.bit.GPIO29 = 1; }


//���ſ��� ͬ���˿ڿ���
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
