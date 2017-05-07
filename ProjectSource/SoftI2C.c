

/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:SoftI2C.c
*文件标识:
*创建日期： 2015年12月21日
*摘要:根据以下基础，改写为通用性质软件模拟I2C。
*2015/12/21: 适应性修改 CC2530 作为32M主频， 单指令周期模式，速度很快。
*当前版本:1.0
*作者: FreeGo
*取代版本:2005-6-19 日期： 创建人：陈曦
*作者:
*完成时间:
************************************************************/
/***********************************************************************
I2C 型号： 总线驱动 I2C 模块名：
2005-6-15 日期： 创建人：陈曦
2005-6-19 日期： 修改人：陈曦
功能描述：
能 ， 此模块包括发送数据及接收数据，应答位发送，并提供了几个直接面对器件的操作函数
很
方便的与用户程序进行连接并扩展。
！ ！ 对高晶振频率要做一定的修改 ， 脉冲 SCL 函数是采用延时方法产生 ， 需要注意的是
！ ！ ！ 的时候一定要延时 E2PROM 在写
说明：
晶振频率要小于 1us 12MHz 机器周期，
则操作失败。 0 则操作成功，返回 1 返回
为器件子地址。 suba 为器件从地址， sla
*******************************************************************************
******/
#include "DeviceIO.h"


#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "F2806x_Examples.h"   // F2806x Examples Include File

#include "SoftI2C.h"
#include "DeviceIO.h"
//
#define  _Nop() {DSP28x_usDelay(2);}
#define  SomeNOP() {DSP28x_usDelay(8);} //定义空指令 //

#define  SDA   GpioDataRegs.GPBDAT.bit.GPIO32 //数据传输位 I2C 模拟 //
#define  SCL   GpioDataRegs.GPBDAT.bit.GPIO33  //时钟控制位 I2C 模拟 //
#define  SET_SDA_IN()  { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0; EDIS;}
#define  SET_SDA_OUT() { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1; EDIS;} //输出置1
#define  SET_SCL_IN()  { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0; EDIS;}
#define  SET_SCL_OUT() { EALLOW; GpioCtrlRegs.GPBDIR.bit.GPIO33 = 1; EDIS;}


uint8_t   I2C_Ack = 0; //应答标志位 //



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

	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;//默认启动内部上拉
	GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;
	GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 2;//质量控制
	EDIS;

  SET_SDA_IN();
  SET_SCL_OUT();
  SCL = 1;
  I2C_Ack = 0;
}
/************************************I2C_Start ************************************
函数名：void  I2C_Start()
入口：
出口：
功能描述：启动I2C 总线，即发送 I2C 初始条件
解释 :在I2C 总线协议中规定的起始位格式是 :在SDL 高电平期间,  SDL从高到低的电平
跳变，它与其他数据格式的区别在于，协议中规定有效位的数据必须在SCL高电平期间保持不变，
只有SCL的低电平期间才能发生跳变，所以这一有别于其他格式的数据才能作为起始位。
调用函数：
全局变量：
********************************************************************************/
 void I2C_Start(void)
{
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  SDA = 1;  // 发送起始条件的数据信号
  _Nop();
  SCL = 1;
  SomeNOP();//起始条件建立时间大于4.7us, 延时
  SomeNOP();
  SomeNOP();
  SDA = 0;  //发送起始信号
//  GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;
  SomeNOP(); //延时 , 起始条件建立时间大于 4us
  SCL = 0;   // 钳住I2C总线准备发送或接收数据
//  GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1;
  //解释：I2C总线空闲状态下都是被上拉为高电平的，所以当他们为低电平时表示忙的状态。
  _Nop();
  _Nop();
}
/************************************************************************
函数名：void  I2C_Stop()
入口：
出口：
结束条件 I2C 总线，即发送 I2C 功能描述：结束
由低电平向高电平 ,SDA 高电平期间 SCL 结束条件的格式是在, 同起始条件的格式类似 : 解释
. 跳变
调用函数：
全局变量：
*****************************************************************************/
void I2C_Stop(void)
{
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  SDA = 0; //发送结束条件的数据信号
  SomeNOP(); //  结束条件建立时间大于 4us,延时
  SomeNOP();
  SomeNOP();
  _Nop();
  SCL = 1;//发送结束条件的时钟信号
  SomeNOP(); //  结束条件建立时间大于 4us,延时
  SomeNOP();
  SomeNOP();
  SDA = 1;//发送I2C 总线结束信号
  SomeNOP();
  SomeNOP();
  SomeNOP();

  SET_SCL_IN();
  SET_SDA_IN();
}

/************************************************************************
函数名：uint8_t I2C_CheckAck(void)
入口：
出口：   0（无应答） ，   1（有应答）
功能描述：
检验I2C的应答信号，有应答则返回1，否则返回0.超时值取255
解释：I2C 总线协议中规定传输的每个字节之后必须跟一个应答位，所以 从器件在接收每个字节后
必须反馈一个应答信号交给主控制器，而主控制器就需要检测从器件回传的应答信号，根据其信息
做出相应的处理，另外，主从之别是相对的，提供SCL信号的为主。

再看应答信号的格式：在由发送器产生的时钟脉冲响应周期里，发送器先释放SDA（置高），然后由
接收器件将SDA拉低，并在这个时钟脉冲周期的高电平期间保持稳定的低电平，即表示从器件做出了
应答。

调用函数： void I2C_Stop()
全局变量：
********************************************************************************/
uint8_t I2C_CheckAck(void)
{
  uint16_t errtime = 1000; // 因故障接收方无Ack,  超时值为 255
  SET_SDA_IN();
  SET_SCL_IN();
  _Nop();
  SDA = 1;//SDA 发送器先释放
  SomeNOP();
  SCL = 1;
  SomeNOP();
  while(SDA)          //  判断 SDA 是否被拉低
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
 函数名：void I2C_SendB(uint8_t c)
入口：uint8_t 型数据
出口：
功能描述：
字节数据传送函数，将数据C发送出去，可以使地址或数据，发送后等待应答，并对此状态为进行操作

注意：在传送数据时，数据（SDA）的改变只能发生在SCL低电平期间，在SCL的高电平期间保持不变。
调用函数： bit I2C_CheckAck()
局变量：I2C_Ack 全
*******************************************************************************/
void I2C_SendB(uint8_t c)
{
  uint8_t BitCnt;
  SET_SDA_OUT();
  SET_SCL_OUT();
  _Nop();
  for (BitCnt = 0; BitCnt < 8; BitCnt++) //8位数据长度
  {
     // 判断发送位 （从高位起发送 ）
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
    SCL = 1;//置时钟线为高通知被控器开始接收数据位
    SomeNOP(); // 保证时钟高电平周期大于 4us
    SCL = 0;
  }
  _Nop();
  _Nop();
  //  检验应答信号 , 作为发送方,所以要检测接收器反馈的应答

  I2C_Ack =  I2C_CheckAck();
  _Nop();
  _Nop();
}
/***********************************************************************
函数名： uint8_t I2C_RcvB()
入口：
出口：型数据 uint8_t
功能描述：
接收从器件传来的数据，并判断总线错误（不发应答信号），发送完后需要调用应答函数。
调用函数：
全局变量：
**********************************************************************************/
uint8_t I2C_RcvB()
{
  uint8_t retc;
  uint8_t BitCnt; // 位


  SET_SCL_OUT();
  _Nop();
  retc = 0;
  //SDA = 1; //.置数据总线为输入方式 作为接收方要释放 = SDA
  SET_SDA_IN();

  for(BitCnt = 0; BitCnt < 8; BitCnt++)
  {
    SCL = 1;//
    _Nop();
    retc = retc<<1;
    if (1 == SDA)
    {
      retc = retc + 1;//中 retc 读数据位，接收的数据放入
    }

    SCL = 0;
    _Nop();
  }

  return(retc);
}


/************************************I2C_Ackn ********************************
函数名： void I2C_Ackn(bit a)
入口：1 或 0
出口：
功能描述：主控制器进行应答信号（可以是应答或非应答信号）
说明 ：作为接收方的时候，必须根据当前自己的状态向发送器反馈应答信号,
调用函数：
全局变量：
*****************************************************************************/
void I2C_Ackn(uint8_t a)
{
  SET_SCL_OUT();
  SET_SDA_OUT();
  _Nop();
  if (a == 0) //在此发送应答或非应答信号 //
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
  SomeNOP(); //时钟电平周期大于4 us //
  SCL = 0;// 总线以便继续接收 I2C 清时钟线钳住
  _Nop();
}
/********************************I2C_ISendB ************************************
函数名：bit I2C_ISendB( uint8_t sla,uint8_t suba,uint8_t c)
入口：从器件地址sla, 子地址suba, 发送字节c.
 出口：1（操作成功）  ， ） 0（操作有误）
功能描述：从启动总线到发送地址，数据，结束总线的全过程，
         如果返回1，表示操作成功，否则操作有误。
调用函数：I2C_Stop() I2C_Start(),I2C_SendB(uint8_t c)
 全局变量：I2C_Ack
***************************************************************************/
 uint8_t I2C_ISendB( uint8_t sla, uint8_t suba, uint8_t c)
{
  I2C_Start(); //启动总线 //
  I2C_SendB(sla); //发送器件地址 //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(suba); //发送器件子地址 //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(c); //发送数据 //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_Stop(); //结束总线 //
  return(1);
}
/**********************************I2C_IRcvB ***********************************
 函数名：bit I2C_IRcvB( uint8_t sla,uint8_t suba,uint8_t* c)
入口：从器件地址sla, 子地址suba, 发送字节c.
 出口：1（操作成功）  ， ） 0（操作有误）
功能描述：从启动总线到发送地址，数据，结束总线的全过程，
         如果返回1，表示操作成功，否则操作有误。
调用函数：I2C_Stop() I2C_Start(),I2C_SendB(uint8_t c) ,I2C_RcvB()
 全局变量：I2C_Ack
**********************************************************************************/
uint8_t I2C_IRcvB( uint8_t sla,  uint8_t suba, uint8_t *c)
{
  I2C_Start(); //启动总线 //
  I2C_SendB(sla);
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_SendB(suba); //发送器件子地址 //
  if(!I2C_Ack)
  {
    return(0);
  }
  I2C_Start(); //重复起始条件 //
  I2C_SendB(sla+1); //发送读操作的地址 //
  if(!I2C_Ack)
  {
    return(0);
  }
  *c =  I2C_RcvB(); //读取数据 //
  I2C_Ackn(1); //发送非应答位 //
  I2C_Stop(); //结束总线 //
  return(1);
}


 /**
  * EEPROM按字节写
  * @param deviceAddr 设备地址
  * @param hightAddr 高位字节地址
  * @param lowAddr 低字节地址
  * @param data  数据
  *
  * @return 0-异常 1-正常
  *
  * @brief   按字节写
  */
uint8_t EEPROMWriteByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t data)
{

	   SoftI2CInit();
	   I2C_Start(); //启动总线 //
	   I2C_SendB(deviceAddr & 0xFE); //设备地址

	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(hightAddr); // 高位字节地址
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(lowAddr); //低字节地址
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(data); //data
	   if(!I2C_Ack)
	   {
	     return(0);
	   }

	   I2C_Stop(); //结束总线 //

	   return(1);

}
/**
 * EEPROM 读
 * @param deviceAddr 设备地址
 * @param hightAddr 高位字节地址
 * @param lowAddr 低字节地址
 * @param pData  读取数据指针
 * @brief   按字节写
 */
uint8_t b1 = 0, b2  = 0;
uint8_t EEPROMReadByte(uint8_t deviceAddr, uint8_t hightAddr, uint8_t lowAddr, uint8_t* pData)
{
	SoftI2CInit();

	I2C_Start(); //启动总线 //
	   I2C_SendB(deviceAddr & 0xFE);
	   if(!I2C_Ack)
	   {
	     return(0);
	   }

	   I2C_SendB(hightAddr); //发送低8Bit
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_SendB(lowAddr); //发送低8Bit
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   I2C_Start(); //重复起始条件 //
	   I2C_SendB(deviceAddr | 0x01); //发送读命令
	   if(!I2C_Ack)
	   {
	     return(0);
	   }
	   b1 =  I2C_RcvB(); //读取数据 //
	   I2C_Ackn(0);      //发送应答位 //
	   I2C_Stop();       //结束总线 //
	   *pData = b1;
	   return(1);


}







