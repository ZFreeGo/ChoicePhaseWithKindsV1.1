#include "DeviceIO.h"




static void delay(unsigned long i);

unsigned char a, b, c,d,temp,count;
unsigned long address, i,j,k,l;
unsigned char stack[64];


void I2C_START()		//I2C 启动
{
	SET_SDAA_H;
	SET_SCLA_H;
	SET_SDAA_L;
	SET_SCLA_L;
}

void I2C_STOP()		//I2C 结束
{
	SET_SDAA_L;
	SET_SCLA_H;
	SET_SDAA_H;
}

void I2C_WRITE_BYTE()	//I2C 写入字节数据，上升沿输入
{
	for(count=0;count<8;count++)
	{
	SET_SCLA_L;
	SDA=temp&0x80;
	SET_SCLA_H;
	SET_SCLA_L;
	temp=temp<<1;
    }
    SET_SDAA_L;
	SET_SCLA_H;
    SET_SCLA_L;
	SET_SDAA_H;
}


void I2C_READ_BYTE()		//I2C 读取数据，上升沿后读取
{
	for(count=0;count<8;count++)
	{
	SET_SDAA_H;
	temp=temp<<1;
	SET_SCLA_H;;
	temp=temp|SDA;
	SET_SCLA_L;;
	}
}

void I2C_SEND_ACK()		//I2C 发出应答
{
	SET_SDAA_L;;
	SET_SCLA_H;;
	SET_SCLA_L;;
}

void I2C_ADDRESS_WRITE()		//I2C 写入的第一个地址位的转换，因为1024的头5个输入固定为10100，然后两位地址，接着读写位，所以要进行变换
{
		temp=0xa0;
		for(count=0;count<5;count++)
		{
		SET_SCLA_L;;
		SDA=temp&0x80;
		SET_SCLA_H;;
		SET_SCLA_L;;
		temp=temp<<1;
    	}

		SET_SCLA_L;;
		SDA=address&0x200000;
		SET_SCLA_H;;
		SET_SCLA_L;;

		SET_SCLA_L;;
		SDA=address&0x10000;
		SET_SCLA_H;;
		SET_SCLA_L;;

		SET_SCLA_L;;
		SET_SDAA_L;;
		SET_SCLA_H;;
		SET_SCLA_L;;

	    SET_SDAA_L;;
		SET_SCLA_H;;
	    SET_SCLA_L;;
		SET_SDAA_H;
}

void I2C_ADDRESS_READ()		//I2C 读取的第一个地址位的转换， 因为1024的头5位固定为10100，然后两位地址位，接着是读写位，所有要进行变换
{
		temp=0xa0;
		for(count=0;count<5;count++)
		{
		SET_SCLA_L;;
		SDA=temp&0x80;
		SET_SCLA_H;;
		SET_SCLA_L;;
		temp=temp<<1;
    	}

		SET_SCLA_L;;
		SDA=address&0x200000;
		SET_SCLA_H;;
		SET_SCLA_L;;

		SET_SCLA_L;;
		SDA=address&0x10000;
		SET_SCLA_H;;
		SET_SCLA_L;;

		SET_SCLA_L;;
		SET_SDAA_H;
		SET_SCLA_H;;
		SET_SCLA_L;;

	    SET_SDAA_L;;
		SET_SCLA_H;;
	    SET_SCLA_L;;
		SET_SDAA_H;

}

void I2C_WRITE_DATA(unsigned long address, unsigned char d) 		//按指定的地址和长度，把堆栈stack[]中的数据存入1024。
{
		I2C_START();

		I2C_ADDRESS_WRITE();

		temp=(address&0xff00)>>8;
		I2C_WRITE_BYTE();

		temp=address&0xff;
		I2C_WRITE_BYTE();

		for(c=0;c<d;c++)
		{
		temp=stack[c];
		I2C_WRITE_BYTE();
		}

		I2C_STOP();
}

void I2C_READ_DATA(unsigned long address,unsigned char d)		//按指定的地址和长度，把堆栈stack[]中的数据读出。
{
		I2C_START();

		I2C_ADDRESS_WRITE();

		temp=(address&0xff00)>>8;
		I2C_WRITE_BYTE();

		temp=address&0xff;
		I2C_WRITE_BYTE();

		I2C_START();

		I2C_ADDRESS_READ();


		for(b=0;b<d;b++)
		{
		I2C_READ_BYTE();
		stack[b]=temp;
		I2C_SEND_ACK();
		}
		I2C_READ_BYTE();

		I2C_STOP();
}


void TestEeprom (void)
{

		SDA = 1;
		SCL = 1;

        SCON = 0x50;	//REN=1允许串行接受状态，串口工作模式2
	    TMOD|= 0x20;	//定时器工作方式2
		PCON|= 0x80;	//波特率提高一倍
	    TH1 = 0xFf;		//baud*2  /*  波特率57600、数据位8、停止位1。效验位无 (11.0592M)
    	TL1 = 0xFf;
		TR1  = 1;		//开启定时器1

		for(a=0x0;a<64;a++)	 			//先给堆栈stack[]设预值
		{
		stack[a]=a+1;
		temp=stack[a];
		SBUF=temp;
		while(!TI);
		delay(1000);
		}

	   	I2C_WRITE_DATA(0x024600,64);  	//在指定地址处写入规定长度的数据；

		delay(500);						//在芯片写入期间不能读取，强行读取会出现错误，必须延时。

dd:		I2C_READ_DATA(0x024600,64);		//重复在指定的位置读取指定长度的数据，并通过串口进行显示。

		for(c=0;c<64;c++)
		{
		temp=stack[c];
		SBUF=temp;
		while(!TI);
		delay(1000);
		}

goto dd;

}

/**************************************************
		  延时处理程序
**************************************************/

delay(unsigned long i)
{
	while(i--);
}



