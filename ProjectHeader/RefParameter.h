/*
 * RefParameter.h
 *
 *  Created on: 2017��4��4��
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_REFPARAMETER_H_
#define PROJECTHEADER_REFPARAMETER_H_

#include "StdType.h"


/**
 * ���ڵ�ѹ����
 */
#define PHASE_A  0
#define PHASE_B  1
#define PHASE_C  2


/**
 * ������ʱʱ�䶨��
 */
typedef struct TagProcessDelayTime
{
	uint16_t sampleDelay; //������ʱʱ�� us
	uint16_t innerDelay; //�ڲ�������ʱ us
	float calDelay;    //�ڲ�����ó��Ľ��е���ʱ us
	uint16_t transmitDelay;//������ʱ us
	uint16_t actionDelay;//������ʱ--��բ us
	float sumDelay;   //����ʱ= sampleDelay + innerDelay + transmitDelay + actionDelay
	int16_t compensationTime;// ����ʱ�� us


}RefProcessTime;

/**
 * У׼ϵ��
 */
typedef struct TagSystemCalibrationCoefficient
{
	float voltageCoefficient1;//Ĭ������A���ѹУ׼ϵ��
	float voltageCoefficient2;//Ĭ������B���ѹУ׼ϵ��
	float voltageCoefficient3;//Ĭ������C���ѹУ׼ϵ��
	float voltageCoefficient4;//Ĭ�����ڱ��õ�ѹУ׼ϵ��

	float frequencyCoefficient;//Ƶ��У׼ϵ��

}SystemCalibrationCoefficient;
/**
 * �����ѹ����
 */
typedef struct TagSystemVoltageParameter
{
	float voltageA;//Ĭ������A���ѹ ����ֵ
	float voltageB;//Ĭ������B���ѹ
	float VoltageC;//Ĭ������C���ѹ
	float voltage0;//Ĭ�����ڱ��õ�ѹ

	float frequency;//Ƶ�� hz
	float period;//���� us
	float delayAB; //B���ͺ�A����ʱ us
	float delayAC; //���ͺ�A����ʱ us

}SystemVoltageParameter;
/**
 * ������ֵ
 */
typedef struct TagUpDownValue
{
	float upper;//����
	float down;//����

}UpDownValue;

/**
 * ϵͳ�޶�����ֵ
 */
typedef struct TagLimitValue
{
	UpDownValue frequency; //Ƶ��
	UpDownValue workVoltage; //������ѹ
	UpDownValue inportVoltage; //���������ѹ

}LimitValue;


/**
 *��������뻡�ȹ�һ��ֵ
 */
typedef struct TagActionParameter
{
	uint8_t enable; //ʹ�ܱ�־��0-��ֹ������ʹ��
    uint8_t phase; //��A-1,B-2,C-3
    uint16_t  actionRad; //�趨���ȹ�һ��ֵr = M/65536 *2*PI
    float realRad;
    float realRatio;//������Ϊ��Ӧʱ��COS����PI/6 Ϊ1/12
    float realTime; //realRatio * ����

    float startTime;//����ͬ����������� ��ʼʱ��

}ActionRad;
/**
 *ͬ������
 */
typedef struct TagSyncCommand
{
	uint8_t commandByte[8]; //�����ֽ�����
	uint8_t configbyte; //������
	uint16_t actionRadA; //A   �趨���ȹ�һ��ֵr = M/65536 *2*PI
	uint16_t actionRadB; //B
	uint16_t actionRadC; //C
}SyncCommand;


/**
 * ��������
 */
typedef struct TagConfigData
{
    uint8_t ID; //ID��
    void* pData;//ָ������
    uint8_t type;//����
    void (*fSetValue)(PointUint8*, struct TagConfigData* )  ;
    void (*fGetValue)(PointUint8*, struct TagConfigData* )  ;
}ConfigData;


/**
 * ���ⲿ���õ�ȫ�ֱ���
 */
extern RefProcessTime g_ProcessDelayTime[3];
extern SystemCalibrationCoefficient g_SystemCalibrationCoefficient;
extern SystemVoltageParameter g_SystemVoltageParameter;
extern ActionRad g_PhaseActionRad[3];

extern void RefParameterInit(void);
#endif /* PROJECTHEADER_REFPARAMETER_H_ */
