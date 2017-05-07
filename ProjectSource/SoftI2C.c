

/***********************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:SoftI2C.c
*�ļ���ʶ:
*�������ڣ� 2015��12��21��
*ժҪ:�������»�������дΪͨ���������ģ��I2C��
*2015/12/21: ��Ӧ���޸� CC2530 ��Ϊ32M��Ƶ�� ��ָ������ģʽ���ٶȺܿ졣
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:2005-6-19 ���ڣ� �����ˣ�����
*����:
*���ʱ��:
************************************************************/
/***********************************************************************
I2C �ͺţ� �������� I2C ģ������
2005-6-15 ���ڣ� �����ˣ�����
2005-6-19 ���ڣ� �޸��ˣ�����
����������
�� �� ��ģ������������ݼ��������ݣ�Ӧ��λ���ͣ����ṩ�˼���ֱ����������Ĳ�������
��
��������û�����������Ӳ���չ��
�� �� �Ը߾���Ƶ��Ҫ��һ�����޸� �� ���� SCL �����ǲ�����ʱ�������� �� ��Ҫע�����
�� �� �� ��ʱ��һ��Ҫ��ʱ E2PROM ��д
˵����
����Ƶ��ҪС�� 1us 12MHz �������ڣ�
�����ʧ�ܡ� 0 ������ɹ������� 1 ����
Ϊ�����ӵ�ַ�� suba Ϊ�����ӵ�ַ�� sla
*******************************************************************************
******/
#include "DeviceIO.h"


#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "F2806x_Examples.h"   // F2806x Examples Include File

#include "SoftI2C.h"
#include "DeviceIO.h"
//
#define  _Nop() {DSP28x_usDelay(2);}
#define  SomeNOP() {DSP28x_usDelay(8);} //�����ָ�� //

#define  SDA   GpioDataRegs.GPBDAT.bit.GPIO32 //���ݴ���λ I2C ģ�� //
#define  SCL   GpioDataRegs.GPBDAT.bit.GPIO33  //ʱ�ӿ���λ I2C ģ�� //
#define  SET_SDA_IN()  { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0; EDIS;}
#define  SET_SDA_OUT() { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1; EDIS;} //�����1
#define  SET_SCL_IN()  { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0; EDIS;}
#define  SET_SCL_OUT() { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1; EDIS;}


uint8_t   I2C_Ack = 0; //Ӧ���־λ //



void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_CheckAck(void);
void I2C_SendB(uint8_t c);
uint8_t I2C_RcvB(void) ;
void I2C_Ackn(uint8_t a) ;

uint16_t I2C_RcvB_test(void);



void SoftI2CInit(void)
{
	 EALLOW;
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;

	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;//Ĭ�������ڲ�����
	GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 2;//��������
	EDIS;

  SET_SDA_IN();
  SET_SCL_OUT();
  SCL = 1;
  I2C_Ack = 0;
}
/************************************I2C_Start ************************************
��������void  I2C_Start()
��ڣ�
���ڣ�
��������������I2C ���ߣ������� I2C ��ʼ����
���� :��I2C ����Э���й涨����ʼλ��ʽ�� :��SDL �ߵ�ƽ�ڼ�,  SDL�Ӹߵ��͵ĵ�ƽ
���䣬�����������ݸ�ʽ���������ڣ�Э���й涨��Чλ�����ݱ�����SCL�ߵ�ƽ�ڼ䱣�ֲ��䣬
ֻ��SCL�ĵ͵�ƽ�ڼ���ܷ������䣬������һ�б���������ʽ�����ݲ�����Ϊ��ʼλ��
���ú�����
ȫ�ֱ�����
********************************************************************************/
 void I2C_Start(void)
{
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  SDA = 1;  // ������ʼ�����������ź�
  _Nop();
  SCL = 1;
  SomeNOP();//��ʼ��������ʱ�����4.7us, ��ʱ
  SomeNOP();
  SomeNOP();
  SDA = 0;  //������ʼ�ź�
//  GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;
  SomeNOP(); //��ʱ , ��ʼ��������ʱ����� 4us
  SCL = 0;   // ǯסI2C����׼�����ͻ��������
//  GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;
  //���ͣ�I2C���߿���״̬�¶��Ǳ�����Ϊ�ߵ�ƽ�ģ����Ե�����Ϊ�͵�ƽʱ��ʾæ��״̬��
  _Nop();
  _Nop();
}
/************************************************************************
��������void  I2C_Stop()
��ڣ�
���ڣ�
�������� I2C ���ߣ������� I2C ��������������
�ɵ͵�ƽ��ߵ�ƽ ,SDA �ߵ�ƽ�ڼ� SCL ���������ĸ�ʽ����, ͬ��ʼ�����ĸ�ʽ���� : ����
. ����
���ú�����
ȫ�ֱ�����
*****************************************************************************/
void I2C_Stop(void)
{
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  SDA = 0; //���ͽ��������������ź�
  SomeNOP(); //  ������������ʱ����� 4us,��ʱ
  SomeNOP();
  SomeNOP();
  _Nop();
  SCL = 1;//���ͽ���������ʱ���ź�
  SomeNOP(); //  ������������ʱ����� 4us,��ʱ
  SomeNOP();
  SomeNOP();
  SDA = 1;//����I2C ���߽����ź�
  SomeNOP();
  SomeNOP();
  SomeNOP();

  SET_SCL_IN();
  SET_SDA_IN();
}

/************************************************************************
��������uint8_t I2C_CheckAck(void)
��ڣ�
���ڣ�   0����Ӧ�� ��   1����Ӧ��
����������
����I2C��Ӧ���źţ���Ӧ���򷵻�1�����򷵻�0.��ʱֵȡ255
���ͣ�I2C ����Э���й涨�����ÿ���ֽ�֮������һ��Ӧ��λ������ �������ڽ���ÿ���ֽں�
���뷴��һ��Ӧ���źŽ���������������������������Ҫ���������ش���Ӧ���źţ���������Ϣ
������Ӧ�Ĵ������⣬����֮������Եģ��ṩSCL�źŵ�Ϊ����

�ٿ�Ӧ���źŵĸ�ʽ�����ɷ�����������ʱ��������Ӧ��������������ͷ�SDA���øߣ���Ȼ����
����������SDA���ͣ��������ʱ���������ڵĸߵ�ƽ�ڼ䱣���ȶ��ĵ͵�ƽ������ʾ������������
Ӧ��

���ú����� void I2C_Stop()
ȫ�ֱ�����
********************************************************************************/
uint8_t I2C_CheckAck(void)
{
  uint16_t errtime = 1000; // ����Ͻ��շ���Ack,  ��ʱֵΪ 255
  SET_SDA_IN();
  SET_SCL_IN();
  _Nop();
  SDA = 1;//SDA ���������ͷ�
  SomeNOP();
  SCL = 1;
  SomeNOP();
  while(SDA)          //  �ж� SDA �Ƿ�����
  {

    errtime--;
    if(errtime == 0)
    {
      I2C_Stop();
      return(0);
    }
    SomeNOP();
  }
  SCL = 0;
  _Nop();
  return(1);
}

/****************************************************************************
 ��������void I2C_SendB(uint8_t c)
��ڣ�uint8_t ������
���ڣ�
����������
�ֽ����ݴ��ͺ�����������C���ͳ�ȥ������ʹ��ַ�����ݣ����ͺ�ȴ�Ӧ�𣬲��Դ�״̬Ϊ���в���

ע�⣺�ڴ�������ʱ�����ݣ�SDA���ĸı�ֻ�ܷ�����SCL�͵�ƽ�ڼ䣬��SCL�ĸߵ�ƽ�ڼ䱣�ֲ��䡣
���ú����� bit I2C_CheckAck()
�ֱ�����I2C_Ack ȫ
*******************************************************************************/
void I2C_SendB(uint8_t c)
{
  uint8_t BitCnt;
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  for (BitCnt = 0; BitCnt < 8; BitCnt++) //8λ���ݳ���
  {
     // �жϷ���λ ���Ӹ�λ���� ��
    if((c<<BitCnt) & 0x80)
    {
      SDA = 1;
    }
    else
    {
    	GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;
      SDA = 0;
    }
    _Nop(); //
    SCL = 1;//��ʱ����Ϊ��֪ͨ��������ʼ��������λ
    SomeNOP(); // ��֤ʱ�Ӹߵ�ƽ���ڴ��� 4us
    SCL = 0;
  }
  _Nop();
  _Nop();
  //  ����Ӧ���ź� , ��Ϊ���ͷ�,����Ҫ��������������Ӧ��

  I2C_Ack =  I2C_CheckAck();
  _Nop();
  _Nop();
}
/***********************************************************************
�������� uint8_t I2C_RcvB()
��ڣ�
���ڣ������� uint8_t
����������
���մ��������������ݣ����ж����ߴ��󣨲���Ӧ���źţ������������Ҫ����Ӧ������
���ú�����
ȫ�ֱ�����
**********************************************************************************/
uint8_t I2C_RcvB()
{
  uint8_t retc;
  uint8_t BitCnt; // λ


  SET_SCL_OUT();
  _Nop();
  retc = 0;
  //SDA = 1; //.����������Ϊ���뷽ʽ ��Ϊ���շ�Ҫ�ͷ� = SDA
  SET_SDA_IN();

  for(BitCnt = 0; BitCnt < 8; BitCnt++)
  {
    SCL = 1;//
    _Nop();
    retc = retc<<1;
    if (1 == SDA)
    {
      retc = retc + 1;//�� retc ������λ�����յ����ݷ���
    }

    SCL = 0;
    _Nop();
  }

  return(retc);
}


/************************************I2C_Ackn ********************************
�������� void I2C_Ackn(bit a)
��ڣ�1 �� 0
���ڣ�
����������������������Ӧ���źţ�������Ӧ����Ӧ���źţ�
˵�� ����Ϊ���շ���ʱ�򣬱�����ݵ�ǰ�Լ���״̬����������Ӧ���ź�,
���ú�����
ȫ�ֱ�����
*****************************************************************************/
void I2C_Ackn(uint8_t a)
{
  SET_SCL_OUT();
  SET_SDA_OUT();
  _Nop();
  if (a == 0) //�ڴ˷���Ӧ����Ӧ���ź� //
  {
    SDA = 0;
  }
  else
  {
    SDA = 1;
  }

   _Nop();
   _Nop();

  SCL = 1;
  SomeNOP();
  SomeNOP(); //ʱ�ӵ�ƽ���ڴ���4 us //
  SCL = 0;// �����Ա�������� I2C ��ʱ����ǯס
  _Nop();
}
/********************************I2C_ISendB ************************************
��������bit I2C_ISendB( uint8_t sla,uint8_t suba,uint8_t c)
��ڣ���������ַsla, �ӵ�ַsuba, �����ֽ�c.
 ���ڣ�1�������ɹ���  �� �� 0����������
�������������������ߵ����͵�ַ�����ݣ��������ߵ�ȫ���̣�
         �������1����ʾ�����ɹ��������������
���ú�����I2C_Stop() I2C_Start(),I2C_SendB(uint8_t c)
 ȫ�ֱ�����I2C_Ack
***************************************************************************/
 uint8_t I2C_ISendB( uint8_t sla, uint8_t suba, uint8_t c)
{
  I2C_Start(); //�������� //
  I2C_SendB(sla); //����������ַ //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(suba); //���������ӵ�ַ //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(c); //�������� //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_Stop(); //�������� //
  return(1);
}
/**********************************I2C_IRcvB ***********************************
 ��������bit I2C_IRcvB( uint8_t sla,uint8_t suba,uint8_t* c)
��ڣ���������ַsla, �ӵ�ַsuba, �����ֽ�c.
 ���ڣ�1�������ɹ���  �� �� 0����������
�������������������ߵ����͵�ַ�����ݣ��������ߵ�ȫ���̣�
         �������1����ʾ�����ɹ��������������
���ú�����I2C_Stop() I2C_Start(),I2C_SendB(uint8_t c) ,I2C_RcvB()
 ȫ�ֱ�����I2C_Ack
**********************************************************************************/
uint8_t I2C_IRcvB( uint8_t sla,  uint8_t suba, uint8_t *c)
{
  I2C_Start(); //�������� //
  I2C_SendB(sla);
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(suba); //���������ӵ�ַ //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_Start(); //�ظ���ʼ���� //
  I2C_SendB(sla+1); //���Ͷ������ĵ�ַ //
  if(!I2C_Ack)
  {
    return(0);
  }
  *c =  I2C_RcvB(); //��ȡ���� //
  I2C_Ackn(1); //���ͷ�Ӧ��λ //
  I2C_Stop(); //�������� //
  return(1);
}


 /**
  * EEPROM���ֽ�д
  * @param deviceAddr �豸��ַ
  * @param hightAddr ��λ�ֽڵ�ַ
  * @param lowAddr ���ֽڵ�ַ
  * @param data  ����
  *
  * @return 0-�쳣 1-����
  *
  * @brief   ���ֽ�д
  */
uint8_t EEPROMWriteByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t data)
{

	   SoftI2CInit();
	   I2C_Start(); //�������� //
	   I2C_SendB(deviceAddr & 0xFE); //�豸��ַ

	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(hightAddr); // ��λ�ֽڵ�ַ
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(lowAddr); //���ֽڵ�ַ
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(data); //data
	   if(!I2C_Ack)
	   {
	     return(0);
	   }

	   I2C_Stop(); //�������� //

	   return(1);

}
/**
 * EEPROM ��
 * @param deviceAddr �豸��ַ
 * @param hightAddr ��λ�ֽڵ�ַ
 * @param lowAddr ���ֽڵ�ַ
 * @param pData  ��ȡ����ָ��
 * @brief   ���ֽ�д
 */
uint8_t b1 = 0, b2  = 0;
uint8_t EEPROMReadByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t* pData)
{
	SoftI2CInit();

	I2C_Start(); //�������� //
	   I2C_SendB(deviceAddr & 0xFE);
	   if(!I2C_Ack)
	   {
	     return(0);
	   }

	   I2C_SendB(hightAddr); //���͵�8Bit
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(lowAddr); //���͵�8Bit
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_Start(); //�ظ���ʼ���� //
	   I2C_SendB(deviceAddr | 0x01); //���Ͷ�����
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   b1 =  I2C_RcvB(); //��ȡ���� //
	   I2C_Ackn(0);      //����Ӧ��λ //
	   I2C_Stop();       //�������� //
	   *pData = b1;
	   return(1);


}







