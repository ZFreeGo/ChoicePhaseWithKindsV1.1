/*
 * RefParameter.h
 *
 *  Created on: 2017��4��4��
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_REFPARAMETER_H_
#define PROJECTHEADER_REFPARAMETER_H_

#include "StdType.h"
#include "DeviceNet.h"

//EEPROM��ַ
#define EEPROM_ADDRESS  0xA0

/**
 * ���ڵ�ѹ����
 */
#define PHASE_A  0
#define PHASE_B  1
#define PHASE_C  2


#define WOKE_WATCH  0xA1
#define WOKE_CONFIG 0xAA
#define WOKE_NORMAL 0x55

#define PULSE_COUNT 10 //�������


#define CAL_FACTOR 0.2098083496094 //���ۼ���ϵ��

//Ƶ�ʺϼ�
typedef struct RagFreqCollect
{
	float FreqInit;//��ʼƵƵ������
	float FreqReal;//��ǰʵʱƵ��
	float FreqMean;//�¼���ʵʱƵ��
	float FreqCal;
} FreqCollect;

/**
 * ������ʱʱ�䶨��
 */
typedef struct TagProcessDelayTime
{
	uint16_t sampleDelay; //������ʱʱ�� us
	uint16_t innerDelay; //�ڲ�������ʱ us
	uint16_t pulseDelay;  //ͬ������������ʱus (400us)
	float calDelay;    //�ڲ�����ó��Ľ��е���ʱ us
	float calDelayCheck;    //У�����ڲ�����ó��Ľ��е���ʱ us
	uint16_t transmitDelay;//������ʱ us
	uint16_t actionDelay;//������ʱ--��բ us
	float sumDelay;   //����ʱ= sampleDelay + innerDelay + transmitDelay + actionDelay  + pulseDelay
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
	float voltageC;//Ĭ������C���ѹ
	float voltage0;//Ĭ�����ڱ��õ�ѹ

	FreqCollect frequencyCollect;//Ƶ�� hz
	float period;//���� us
	uint16_t delayAB; //B���ͺ�A����ʱ us
	uint16_t delayAC; //���ͺ�A����ʱ us

	float workVoltage; //������ѹ



	float perodCap;//ͨ��Ӳ����׽����ʵʱ����
	float perodMeanCap;//ͨ��Ӳ����׽����ƽ������

}SystemVoltageParameter;
/**
 * ������ֵ
 */
typedef struct TagMaxMinValue
{
	float max;//����
	float min;//����

}MaxMinValue;

/**
 * ϵͳ�޶�����ֵ
 */
typedef struct TagLimitValue
{
	MaxMinValue frequency; //Ƶ��
	MaxMinValue workVoltage; //������ѹ
	MaxMinValue inportVoltage; //���������ѹ
	uint16_t syncReadyWaitTime;

}LimitValue;


/**
 *��������뻡�ȹ�һ��ֵ
 */
typedef struct TagActionParameter
{
	uint8_t readyFlag; //��Ӧ������Ӧ�� ׼����־
	uint8_t enable; //ʹ�ܱ�־��0-��ֹ������ʹ��
    uint8_t phase; //��A-1,B-2,C-3
    uint16_t  actionRad; //�趨���ȹ�һ��ֵr = M/65536 *2*PI
    float realRad; // ��Ӧ�����һ��ǲ�
    float realDiffRatio;//��ֵ���ʣ������趨ֵ.��ǰһ��Ĳ�ֵ�������ǰһ���ֵΪ0
    float realRatio;//������Ϊ��Ӧʱ��COS����PI/6 Ϊ1/12
    float realTime; //realRatio * ����
    float realDiffTime; //realRatio * ����

    float startTime;//����ͬ����������� ��ʼʱ��
    uint8_t loopByte;//��·������
    uint8_t count; //��������

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
extern LimitValue g_SystemLimit;
extern volatile uint32_t g_CANErrorStatus;
extern uint8_t g_MacList[4];
extern uint8_t g_WorkMode;

extern uint8_t ReadParamValue(uint8_t id, PointUint8* pPoint);
extern uint8_t SetParamValue(uint8_t id, PointUint8* pPoint);
extern void RefParameterInit(void);
extern uint8_t UpdateSystemSetData(void);

extern struct DefFrameData  g_NetSendFrame;
extern uint8_t CheckVoltageStatus(void);
extern uint8_t CheckPhaseVoltageStatus(uint8_t phase);
extern uint8_t CheckFrequencyStatus(void);


#endif /* PROJECTHEADER_REFPARAMETER_H_ */
