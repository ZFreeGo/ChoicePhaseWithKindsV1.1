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
	float sampleDelay; //������ʱʱ�� us
	float innerDelay; //�ڲ���ʱ us
	float transmitDelay;//������ʱ us
	float actionDelay;//������ʱ--��բ us
	float compensationTime;// ����ʱ�� us

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
typedef struct TagLimitValue
{
	float upper;//����
	float down;//����

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

}ActionRad;

/**
 * ���ⲿ���õ�ȫ�ֱ���
 */
extern RefProcessTime g_ProcessDelayTime[3];
extern SystemCalibrationCoefficient g_SystemCalibrationCoefficient;
extern SystemVoltageParameter g_SystemVoltageParameter;
extern ActionRad g_PhaseActionRad[3];


#endif /* PROJECTHEADER_REFPARAMETER_H_ */
