#include "DeviceIO.h"




static void delay(unsigned long i);

unsigned char a, b, c,d,temp,count;
unsigned long address, i,j,k,l;
unsigned char stack[64];


void I2C_START()		//I2C ����
{
	SET_SDAA_H;
	SET_SCLA_H;
	SET_SDAA_L;
	SET_SCLA_L;
}

void I2C_STOP()		//I2C ����
{
	SET_SDAA_L;
	SET_SCLA_H;
	SET_SDAA_H;
}

void I2C_WRITE_BYTE()	//I2C д���ֽ����ݣ�����������
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


void I2C_READ_BYTE()		//I2C ��ȡ���ݣ������غ��ȡ
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

void I2C_SEND_ACK()		//I2C ����Ӧ��
{
	SET_SDAA_L;;
	SET_SCLA_H;;
	SET_SCLA_L;;
}

void I2C_ADDRESS_WRITE()		//I2C д��ĵ�һ����ַλ��ת������Ϊ1024��ͷ5������̶�Ϊ10100��Ȼ����λ��ַ�����Ŷ�дλ������Ҫ���б任
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

void I2C_ADDRESS_READ()		//I2C ��ȡ�ĵ�һ����ַλ��ת���� ��Ϊ1024��ͷ5λ�̶�Ϊ10100��Ȼ����λ��ַλ�������Ƕ�дλ������Ҫ���б任
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

void I2C_WRITE_DATA(unsigned long address, unsigned char d) 		//��ָ���ĵ�ַ�ͳ��ȣ��Ѷ�ջstack[]�е����ݴ���1024��
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

void I2C_READ_DATA(unsigned long address,unsigned char d)		//��ָ���ĵ�ַ�ͳ��ȣ��Ѷ�ջstack[]�е����ݶ�����
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

        SCON = 0x50;	//REN=1�����н���״̬�����ڹ���ģʽ2
	    TMOD|= 0x20;	//��ʱ��������ʽ2
		PCON|= 0x80;	//���������һ��
	    TH1 = 0xFf;		//baud*2  /*  ������57600������λ8��ֹͣλ1��Ч��λ�� (11.0592M)
    	TL1 = 0xFf;
		TR1  = 1;		//������ʱ��1

		for(a=0x0;a<64;a++)	 			//�ȸ���ջstack[]��Ԥֵ
		{
		stack[a]=a+1;
		temp=stack[a];
		SBUF=temp;
		while(!TI);
		delay(1000);
		}

	   	I2C_WRITE_DATA(0x024600,64);  	//��ָ����ַ��д��涨���ȵ����ݣ�

		delay(500);						//��оƬд���ڼ䲻�ܶ�ȡ��ǿ�ж�ȡ����ִ��󣬱�����ʱ��

dd:		I2C_READ_DATA(0x024600,64);		//�ظ���ָ����λ�ö�ȡָ�����ȵ����ݣ���ͨ�����ڽ�����ʾ��

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
		  ��ʱ�������
**************************************************/

delay(unsigned long i)
{
	while(i--);
}



