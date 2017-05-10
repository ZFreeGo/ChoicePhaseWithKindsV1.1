/****************************************************
*Copyright(c) 2015,FreeGo
*��������Ȩ��
*�ļ�����:MonitorCalculate.c
*�ļ���ʶ:
*�������ڣ� 2017��4��4��
*ժҪ:
*2017/4/4:������ż���ʹ�õĸ�������
*��ǰ�汾:1.0
*����: FreeGo
*ȡ���汾:
*����:
*���ʱ��:
*******************************************************/

#include "RefParameter.h"
#include "SoftI2C.h"
#include "BasicModule.h"
#include "DeviceIO.h"
#include "Header.h"



/**
 * ��ȡ��Чλ��
 */
#define GET_ENOB(type) ((uint8_t)type & 0x0F)
/**
* �ֽ���ɸ���
*/
#define GET_BYTE_NUM(type) ((uint8_t)(type>>4) & 0x0F)


#define BIG_MAX 3
#define NORMOL_VALUE 2
#define LESS_MIN 1

/*
 * �ֲ���������
 */
static void SetValueFloat32(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueFloat32(PointUint8* pPoint, ConfigData* pConfig);

static void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
static void SetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueUint8(PointUint8* pPoint, ConfigData* pConfig);
static void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig);

static uint8_t EEPROMReadData(uint8_t hightAddr, uint8_t lowAddr, PointUint8* pPoint);
static uint8_t EEPROMWriteData(uint8_t hightAddr, uint8_t lowAddr, PointUint8* pPoint);

static uint8_t ReadLocalSaveData(uint8_t startId, uint8_t len, uint16_t* pSum);
static uint8_t UpdateLocalSaveData(uint8_t startId, uint8_t len, uint16_t* pSum);

/**
 * ����������Ƶ���ʱ����
 */
RefProcessTime g_ProcessDelayTime[3];

/**
 * ϵͳ����У׼
 */
SystemCalibrationCoefficient g_SystemCalibrationCoefficient;


/**
 * ϵͳ�����ѹ�йز���
 */
SystemVoltageParameter g_SystemVoltageParameter;

/**
 *���ද��ͬ�������趨����
 */
ActionRad g_PhaseActionRad[3];

/**
 *ϵͳ����������
 */
LimitValue g_SystemLimit;

/**
 *����MAC
 */
uint8_t g_LocalMac;
/**
 *����ģʽ
 */
uint8_t g_WorkMode;




/**
 *CAN����
 */
volatile uint32_t g_CANErrorStatus;



uint8_t  BufferData[10];//���ջ�������
struct DefFrameData  g_NetSendFrame; //����֡����

/**
 *����ֵ�ۼӺ�
 */
uint16_t CumulativeSum = 0;


#define PARAMETER_LEN 30  //���ò����б�
#define READONLY_PARAMETER_LEN 15  //��ȡ�����б�
#define READONLY_START_ID 0x41
/**
 *ϵͳ���ò����ϼ�
 */
ConfigData g_SetParameterCollect[PARAMETER_LEN]; //���ò����б�--�ɶ���д
ConfigData g_ReadOnlyParameterCollect[READONLY_PARAMETER_LEN]; //�����ϼ�--ֻ���б�


/**
 * ���ò���
 * @param id      ���ú�
 * @param pPoint  ָ�򱣴����ݵ�ָ��
 *
 * @return ������� 0-û�д��� ��0-�д���
 */
uint8_t SetParamValue(uint8_t id, PointUint8* pPoint)
{

	for(uint8_t i = 0; i < PARAMETER_LEN; i++)
	{
		if(g_SetParameterCollect[i].ID == id)
		{
			//TODO :��Ӵ�������ÿһ��Set/Get���������Ӧ�Ĵ������ݡ�
			g_SetParameterCollect[i].fSetValue(pPoint, g_SetParameterCollect + i);
			if (pPoint->len == 0)
			{
				return 0xF1;
			}

			g_SetParameterCollect[i].fGetValue(pPoint, g_SetParameterCollect + i);
			if (pPoint->len == 0)
			{
				return 0xF2;
			}
			return 0;
		}

	}
	return 0xFF;
}
/**
 * ��ȡ����
 * @param id      ���ú�
 * @param pPoint  ָ�򱣴����ݵ�ָ��
 *
 * @return ������� 0-û�д��� ��0-�д���
 */
uint8_t ReadParamValue(uint8_t id, PointUint8* pPoint)
{
	if (id < READONLY_START_ID) //С��ֻ��ID����Ϊ���ò�������
	{
		for(uint8_t i = 0; i < PARAMETER_LEN; i++)
		{
			if(g_SetParameterCollect[i].ID == id)
			{
				//TODO :��Ӵ�������ÿһ��Get���������Ӧ�Ĵ������ݡ�

				g_SetParameterCollect[i].fGetValue(pPoint, g_SetParameterCollect + i);
				if (pPoint->len == 0)
				{
					return 0xF1;
				}

				return 0;
			}

		}
		return 0xF2;
	}
	else
	{
		for(uint8_t i = 0; i < READONLY_PARAMETER_LEN; i++)
		{
			if(g_ReadOnlyParameterCollect[i].ID == id)
			{
				//TODO :��Ӵ�������ÿһ��et���������Ӧ�Ĵ������ݡ�

				g_ReadOnlyParameterCollect[i].fGetValue(pPoint, g_ReadOnlyParameterCollect + i);
				if (pPoint->len == 0)
				{
					return 0xF3;
				}

				return 0;
			}

		}
		return 0xF4;
	}

	return 0xFF;
}

/**
 * ��ʼ��ֻ��ϵͳ�����ϼ�
 */
static void InitReadonlyParameterCollect(void)
{
	uint8_t index = 0, id = READONLY_START_ID;
	//Uint32 -- ���ε�ѹֵ������6λС��
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageA;
	g_ReadOnlyParameterCollect[index].type = 0x46;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageB;
	g_ReadOnlyParameterCollect[index].type = 0x46;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageC;
	g_ReadOnlyParameterCollect[index].type = 0x46;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltage0;
	g_ReadOnlyParameterCollect[index].type = 0x46;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	//Uint16 --[0,65535]����λus
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.delayAB;
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.delayAC;
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].innerDelay;
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].innerDelay;
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].innerDelay;
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;

	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].calDelay;
	g_ReadOnlyParameterCollect[index].type = 0x42;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].calDelay;
	g_ReadOnlyParameterCollect[index].type = 0x42;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].calDelay;
	g_ReadOnlyParameterCollect[index].type = 0x42;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;

	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_PhaseActionRad[0].startTime;
	g_ReadOnlyParameterCollect[index].type = 0x42;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloat32;


	index++;
	//[0-65.535]
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.workVoltage;
	g_ReadOnlyParameterCollect[index].type = 0x23;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.frequencyCollect.FreqMean;
	g_ReadOnlyParameterCollect[index].type = 0x23;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	if (READONLY_PARAMETER_LEN < index)
	{
		while(1);
	}






}
/**
 * ��ʼ��ϵͳ�����ϼ�
 */
static void InitSetParameterCollect(void)
{
	uint8_t index = 0, id = 1;
	//Uint32 -- ���������뵽���֮���У׼ϵ��������6λС��
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.voltageCoefficient1;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.voltageCoefficient2;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.voltageCoefficient3;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.voltageCoefficient4;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.frequencyCoefficient;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.inportVoltage.max;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.inportVoltage.min;
	g_SetParameterCollect[index].type = 0x46;
	g_SetParameterCollect[index].fSetValue = SetValueFloat32;
	g_SetParameterCollect[index].fGetValue = GetValueFloat32;
	index++;

	//Uint16 --[0-65535]����λus
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].sampleDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].sampleDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].sampleDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].transmitDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].transmitDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].transmitDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].actionDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].actionDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].actionDelay;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	//int16 us
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_A].compensationTime;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_B].compensationTime;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index ++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_ProcessDelayTime[PHASE_C].compensationTime;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	//Uint16 --[0-65535]����λms
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.syncReadyWaitTime;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	//[0-65.535]
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.frequency.max;
	g_SetParameterCollect[index].type = 0x23;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.frequency.min;
	g_SetParameterCollect[index].type = 0x23;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.workVoltage.max;
	g_SetParameterCollect[index].type = 0x23;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.workVoltage.min;
	g_SetParameterCollect[index].type = 0x23;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	//Uint16 ��һ��ֵ ��բ���
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_PhaseActionRad[0].actionRad;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_PhaseActionRad[1].actionRad;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_PhaseActionRad[2].actionRad;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_PhaseActionRad[0].loopByte;
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_LocalMac;
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &CumulativeSum;
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;

	if (PARAMETER_LEN != index)
	{

		while(1)
		{
			ON_LED1; //LED1����ָʾ����
		}
	}

}
/**
 *  ʹ��Ĭ��ֵ���г�ʼ��
 */
void DefaultInit(void)
{
	//A��
	g_ProcessDelayTime[PHASE_A].actionDelay = 0;
	g_ProcessDelayTime[PHASE_A].compensationTime = 0;
	g_ProcessDelayTime[PHASE_A].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_A].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_A].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_A].sumDelay = 0;
	g_ProcessDelayTime[PHASE_A].calDelay = 0;
	//B��
	g_ProcessDelayTime[PHASE_B].actionDelay = 0;
	g_ProcessDelayTime[PHASE_B].compensationTime = 0;
	g_ProcessDelayTime[PHASE_B].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_B].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_B].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_B].sumDelay = 0;
	g_ProcessDelayTime[PHASE_B].calDelay = 0;
	//C��
	g_ProcessDelayTime[PHASE_C].actionDelay = 0;
	g_ProcessDelayTime[PHASE_C].compensationTime = 0;
	g_ProcessDelayTime[PHASE_C].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_C].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_C].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_C].sumDelay = 0;
	g_ProcessDelayTime[PHASE_C].calDelay = 0;

	//У׼������ʼ��
	g_SystemCalibrationCoefficient.frequencyCoefficient = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient1 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient2 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient3 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient4 = 1;


	//ϵͳ��ѹ����
	 g_SystemVoltageParameter.voltageC = 0;
	 g_SystemVoltageParameter.delayAB = 0;
	 g_SystemVoltageParameter.delayAC = 0;
	 g_SystemVoltageParameter.frequencyCollect.FreqInit = 50.0f;
	 g_SystemVoltageParameter.frequencyCollect.FreqReal = 50.0f;
	 g_SystemVoltageParameter.frequencyCollect.FreqMean = 50.0f;
	 g_SystemVoltageParameter.frequencyCollect.FreqCal = 50.0f;
	 g_SystemVoltageParameter.period = 20000;
	 g_SystemVoltageParameter.voltage0 = 0;
	 g_SystemVoltageParameter.voltageA = 0;
	 g_SystemVoltageParameter.voltageB = 0;
	 g_SystemVoltageParameter.workVoltage = 0;

	 //����Ԥ�Ʋ��� realRatio
	 g_PhaseActionRad[0].phase = PHASE_A;
	 g_PhaseActionRad[0].actionRad = 0;
	 g_PhaseActionRad[0].enable = 0xff;
	 g_PhaseActionRad[0].realRatio = 0;
	 g_PhaseActionRad[0].startTime = 0;
	 g_PhaseActionRad[0].realTime = 0;

	 g_PhaseActionRad[1].phase = PHASE_B;
	 g_PhaseActionRad[1].actionRad = 0;
	 g_PhaseActionRad[1].enable = 0xff;
	 g_PhaseActionRad[1].realRatio = 0;
	 g_PhaseActionRad[1].startTime = 0;
	 g_PhaseActionRad[1].realTime = 0;

	 g_PhaseActionRad[2].phase = PHASE_C;
	 g_PhaseActionRad[2].actionRad = 0;
	 g_PhaseActionRad[2].enable = 0xff;
	 g_PhaseActionRad[2].realRatio = 0;
	 g_PhaseActionRad[2].startTime = 0;
	 g_PhaseActionRad[2].realTime = 0;


	 //ϵͳ����������
	 g_SystemLimit.frequency.max = 55.0f;
	 g_SystemLimit.frequency.min = 45.0f;
	 g_SystemLimit.workVoltage.max = 3.5f;
	 g_SystemLimit.workVoltage.min =  3.1f;
	 g_SystemLimit.inportVoltage.min = 80;
	 g_SystemLimit.inportVoltage.max = 250;
	 //ͬ��Ԥ�Ƶȴ�ʱ��
	 g_SystemLimit.syncReadyWaitTime = 3000;


	 //CANͨѶ����״̬
	 g_CANErrorStatus = 0;


	 g_LocalMac = 0x0D;



	 //�������ݷ���
	 g_NetSendFrame.pBuffer = BufferData;
}
/**
 * ��ʼ��ȫ�ֱ�������
 */
uint8_t result = 0;
void RefParameterInit(void)
{
	 uint16_t sum = 0, sumOne = 0;
	 result = 0;
	 InitSetParameterCollect();
	 InitReadonlyParameterCollect();

	 DefaultInit();

	 result = ReadLocalSaveData(1, PARAMETER_LEN-1, &sum);
	 NOP();
	 if (result)
	 {
		 DefaultInit();//������Ĭ��ֵ��ʼ��
		 NOP();
	 }
	 else
	 {
		 //��ȡ�ۼӺ�,���һ��
		 result = ReadLocalSaveData(PARAMETER_LEN, 1, &sumOne);
		 if(result)
		 {
			 DefaultInit();//������Ĭ��ֵ��ʼ��
			 NOP();
		 }
		 else
		 {
			 if (CumulativeSum != sum)
			 {
				 DefaultInit();//������Ĭ��ֵ��ʼ��
				 NOP();
			 }
		 }
	 }


	 g_WorkMode = 0;





}


/**
 * ����ϵͳУ׼ϵ��[0,4294.967295]��Ĭ�ϱ�����λС��;
 * ������У׼ϵ��,���ε�ѹֵ
 * <p>
 * ��float��ʽ�洢����4�ֽڽ��н������������
 * 
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void SetValueFloat32(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >=  4)
	{
		//Todo: ����IDѡ������Чλ��
		float ration = 0.000001f;
	    uint32_t data1 = pPoint->pData[1];
	    uint32_t data2 = pPoint->pData[2];
	    uint32_t data3 = pPoint->pData[3];
	    uint32_t data = (data3 << 24) | (data2 << 16) | (data1 << 8)|pPoint->pData[0] ;

		float result = (float)data *  ration;
	    *(float*)pConfig->pData = result;

		//TODO:У׼ϵ���迼��EEPROM�洢
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
* ��ȡϵͳУ׼ϵ��[0,4294.967295]��Ĭ�ϱ�����λС��;
 * ������У׼ϵ��,���ε�ѹֵ
 * <p>
 * ��float��ʽ�洢����4�ֽڽ��н������������
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void GetValueFloat32(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 4)
	{
		//Todo: ����IDѡ������Чλ��

		float ration = 1000000.0f;
		if (pConfig->type == 0x42)
		{
			ration = 100.0f;
		}


		uint32_t result = (uint32_t)(*(float*)pConfig->pData * ration);
		pPoint->pData[0] = (uint8_t)(result & 0x00FF);
		pPoint->pData[1] = (uint8_t)((result >> 8)& 0x00FF);
		pPoint->pData[2] = (uint8_t)((result >> 16)& 0x00FF);
		pPoint->pData[3] = (uint8_t)((result >> 24)& 0x00FF);
		pPoint->len = 4;
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}


/**
 * ���ò���[0,65.535]��Ĭ�ϱ���3λС��
 * �����ڵ�ѹƵ��,������ѹ��
 * <p>
 * ��float�洢����2�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		//Todo: ����IDѡ������Чλ��
		float ration = 0.001f;
		uint16_t data = pPoint->pData[1];
		data =  (data << 8) | pPoint->pData[0];
		float result = (float)data *  ration;
		*(float*)pConfig->pData = result;

		//TODO:�迼��EEPROM�洢
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
 * ��ȡ����[0,65.535]��Ĭ�ϱ���3λС��
 * �����ڵ�ѹƵ��,������ѹ��
 * <p>
 * ��float�洢����2�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		//Todo: ����IDѡ������Чλ��
		float ration = 1000.0f;

		uint16_t result = (uint16_t)(*(float*)pConfig->pData * ration);
		pPoint->pData[0] = (uint8_t)(result & 0x00FF);
		pPoint->pData[1] = (uint8_t)(result >> 8);
		pPoint->len = 2;
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
 * ����ֵ�����2�ֽ�[0,65535]
 * �����ڵ�ѹ������ʱ,������ʱ����բʱ�䣬ͬ��Ԥ�Ƶȴ�ʱ�䣬
 * <p>
 * ��uint16_t��ʽ�洢����2�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void SetValueUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		uint16_t data = pPoint->pData[1];
		data = (data<<8) | pPoint->pData[0];
		*(uint16_t*)pConfig->pData = data;
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
 * ��ȡֵ�����2�ֽ�[0,65535]
 * �����ڵ�ѹ������ʱ,������ʱ����բʱ�䣬ͬ��Ԥ�Ƶȴ�ʱ�䣬
 * <p>
 * ��uint16_t��ʽ�洢����2�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void GetValueUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		uint16_t data = *(uint16_t*)pConfig->pData;
		pPoint->pData[0] = (uint8_t)(data & 0x00FF);
		pPoint->pData[1] = (uint8_t)(data >> 8);
		pPoint->len = 2;
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
 * ����ֵ�����1�ֽ�[0,255]
 *
 * <p>
 * ��uint8_t��ʽ�洢����1�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 1)
	{
		*(uint8_t*)pConfig->pData = pPoint->pData[0];
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}

/**
 * ��ȡֵ�����2�ֽ�[0,65535]
 * �����ڵ�ѹ������ʱ,������ʱ����բʱ�䣬ͬ��Ԥ�Ƶȴ�ʱ�䣬
 * <p>
 * ��uint8_t��ʽ�洢����1�ֽڽ��н������������
 *
 * @param   pPoint    ָ����������
 * @param   pConfig   ָ��ǰ��������
 *
 * @brif ����ͨѶ��������ʹ��
 */
static void GetValueUint8(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 1)
	{
		pPoint->pData[0] =  *(uint8_t*)pConfig->pData;
		pPoint->len = 1;
	}
	else
	{
		pPoint->len = 0; //��Ϊ0����ʾ�����
	}
}


/**
 * ������д��EEPROMָ���ĵ�ַ,��󳤶�С�ڵ���4��д�����¶�ȡ������֤д����ȷ�ԡ�
 *
 * @param   hightAddr    ���ֽڵ�ַ
 * @param   lowAddr   ���ֽڵ�ַ
 * @param pPoint Ҫ��������ݳ���
 *
 * @return 0--��ȷ  ��0--д�����
 *
 * @brif EEPROMд����
 */

static uint8_t EEPROMWriteData(uint8_t hightAddr, uint8_t lowAddr, PointUint8* pPoint)
{
	uint8_t k = 0, readData = 0, result = 0, overCount = 0;
	if(pPoint->len == 0) //���ܵ���0
	{
		return 0xA0;
	}
	if (pPoint->len > 4) //���ܴ���4
	{
		return 0xA1;
	}
	for( k = 0; k < pPoint->len; k++)
	{
		overCount = 0;
		do
		{
			result = EEPROMWriteByte(EEPROM_ADDRESS, hightAddr, lowAddr + k,  pPoint->pData[k]);
			if(!result )
			{
				if(overCount++ > 3)
				{
					return (0xB0 + k);
				}
				 DelayMs(10);//��ʱ10ms���ٴγ���
			}
			else
			{
				break;		//�����˳�		
			}
		}
		while(1);
		
		DelayMs(50);
		 
		overCount = 0;
		do
		{
			result =  EEPROMReadByte(EEPROM_ADDRESS, hightAddr, lowAddr +k,  &readData);
			if(!result )
			{
				if(overCount++ > 3)
				{
					return (0xC0 + k);
				}
				 DelayMs(10);//��ʱ10ms���ٴγ���
			}
			else
			{
				break;		//�����˳�		
			}
		}
		while(1);
		 if (readData != (pPoint->pData[k] & 0x00FF))//����δ�õĸ�λ
		 {
			 return (0xF0 + k);
		 }
	}
	return 0;

}

/**
 * ��ָ��EEPROM��ַ������
 *
 * @param   hightAddr    ���ֽڵ�ַ
 * @param   lowAddr   ���ֽڵ�ַ
 * @param  pPoint Ҫ��������ݳ��ռ�
 *
 * @return 0--��ȷ  ��0--��ȡ����
 *
 * @brif EEPROM������
 */
static uint8_t EEPROMReadData(uint8_t hightAddr, uint8_t lowAddr, PointUint8* pPoint)
{
	uint8_t i = 0, readData = 0, result = 0, overCount = 0;
	if(pPoint->len == 0) //���ܵ���0
	{
		return 0xA0;
	}
	if (pPoint->len > 4) //���ܴ���4
	{
		return 0xA1;
	}
	for( i = 0; i < pPoint->len; i++)
	{
		overCount = 0;
		do
		{
			result =  EEPROMReadByte(EEPROM_ADDRESS, hightAddr, lowAddr + i,  &readData);
			if(!result )
			{
				if(overCount++ > 3)
				{
					return (0xC0 + i);
				}
				 DelayMs(10);//��ʱ10ms���ٴγ���
			}
			else
			{
				pPoint->pData[i] = readData;
				break;		//�����˳�		
			}
		}
		while(1);		
		DelayMs(1);
	}
	return 0;
}

	PointUint8 tempPoint;
	uint8_t testData[4];
	uint8_t testResult;

/**
 * ��EEPROM�ж�ȡ�趨ֵ,�������ۼӺ�
 *
 * @param startId ��ʼID��
 * @param len  ����
 * @param *pSum �ۼӺ� 
 *
 * @return 0--��ȷ  ��0--��ȡ����
 *
 * @brif ��������
 */
static uint8_t ReadLocalSaveData(uint8_t startId, uint8_t len, uint16_t* pSum)
{
	uint8_t i = 0, k = 0;

	 tempPoint.pData = testData;
	 tempPoint.len = 0;

	 if ((4 * PARAMETER_LEN > 252) && (startId + len <= PARAMETER_LEN ) && (startId > 0))
	 {
		 return 0xA0;
	 }
	
	for(i = startId - 1; i <startId + len - 1; i++)
	{
		tempPoint.len = g_SetParameterCollect[i].type >> 4;//��ȡ�ֽ���
		testResult = EEPROMReadData( 0, 4* (g_SetParameterCollect[i].ID-1), &tempPoint);
		if (testResult)
		{
			return 0xB0 + i;
		}
		else
		{
			g_SetParameterCollect[i].fSetValue(&tempPoint, g_SetParameterCollect + i);
		}
		if (tempPoint.len == 0) //����Ϊ0��˵�����ô��󷵻�
		{
			return 0xE0 + i;
		}
		for(k = 0; k <  tempPoint.len; k++)//�����ۼӺ�,���ֽ��ۼ�
		{
			*pSum += tempPoint.pData[k];
		}
	}
	return 0;
}
/**
 *��ֵд�뱾�ش洢
 *
 * @param startId ��ʼID��
 * @param len  ����
 * @param *pSum �ۼӺ� 
 *
 * @return 0--��ȷ  ��0--��ȡ����
 *
 * @brif ����EEPROm����
 */
static uint8_t UpdateLocalSaveData(uint8_t startId, uint8_t len, uint16_t* pSum)
{
	uint8_t i = 0,  k = 0;

	 tempPoint.pData = testData;
	 tempPoint.len = 4;

	 if ((4 * PARAMETER_LEN > 252) && (startId + len <= PARAMETER_LEN ) && (startId > 0))
	 {
		 return 0xA0;
	 }
	
	for(i = startId-1; i <startId + len -1; i++)
	{		
	
		g_SetParameterCollect[i].fGetValue(&tempPoint, g_SetParameterCollect + i);
		if (tempPoint.len == 0) //����Ϊ0��˵�����ô��󷵻�
		{
			return 0xB0 + i;
		}	
	
		testResult = EEPROMWriteData( 0, 4* (g_SetParameterCollect[i].ID-1), &tempPoint);		
		if (testResult)
		{
			return 0xE0 + i;
		}
		
		for(k = 0; k <  tempPoint.len; k++)//�����ۼӺ�,���ֽ��ۼ�
		{
			*pSum += tempPoint.pData[k];
		}
		
		

	}
	return 0;
}
/**
 *��������
 *
 * @brif ����ϵͳ��������
 */
uint8_t resultUpdate;
uint8_t UpdateSystemSetData(void)
{
	uint16_t sum = 0;

	resultUpdate = UpdateLocalSaveData(1,  PARAMETER_LEN - 1, &sum);
	NOP() ;
	if (!resultUpdate)
	{
		CumulativeSum = sum;
		resultUpdate = UpdateLocalSaveData(PARAMETER_LEN, 1, &sum);//��������У���
		NOP() ;

	}
	NOP() ;
	resultUpdate = ReadLocalSaveData(1, PARAMETER_LEN, &sum);
	return resultUpdate;
}

/**
 * @brif ���ϵͳ��ѹ
 */
uint8_t CheckVoltageStatus(void)
{
	uint8_t statusA = 0, statusB = 0, statusC = 0, status0 = 0;

	//A��
	if (g_SystemVoltageParameter.voltageA <  g_SystemLimit.inportVoltage.min)
	{
		statusA = LESS_MIN;
	}
	else if (g_SystemVoltageParameter.voltageA >  g_SystemLimit.inportVoltage.max)
	{
		statusA = BIG_MAX;
	}
	else
	{
		statusA = NORMOL_VALUE;
	}

	//B��
	if (g_SystemVoltageParameter.voltageB <  g_SystemLimit.inportVoltage.min)
	{
		statusB = LESS_MIN;
	}
	else if (g_SystemVoltageParameter.voltageB >  g_SystemLimit.inportVoltage.max)
	{
		statusB = BIG_MAX;
	}
	else
	{
		statusB = NORMOL_VALUE;
	}

	//C��
	if (g_SystemVoltageParameter.voltageC <  g_SystemLimit.inportVoltage.min)
	{
		statusC = LESS_MIN;
	}
	else if (g_SystemVoltageParameter.voltageC >  g_SystemLimit.inportVoltage.max)
	{
		statusC = BIG_MAX;
	}
	else
	{
		statusC = NORMOL_VALUE;
	}

	//������
	if (g_SystemVoltageParameter.voltage0 <  g_SystemLimit.inportVoltage.min)
	{
		status0 = LESS_MIN;
	}
	else if (g_SystemVoltageParameter.voltage0 >  g_SystemLimit.inportVoltage.max)
	{
		status0 = BIG_MAX;
	}
	else
	{
		status0 = NORMOL_VALUE;
	}

	return ((status0<<6)|(statusC<<4)|(statusB<<2)|(statusA))&0x00FF;


}

/**
 * @brif ���ϵͳƵ��״̬
 */
uint8_t CheckFrequencyStatus(void)
{
	uint8_t statusA = 0;

	//A��
	if (g_SystemVoltageParameter.frequencyCollect.FreqMean < 49)
	{
		statusA = LESS_MIN;
	}
	else if (g_SystemVoltageParameter.frequencyCollect.FreqMean >  51)
	{
		statusA = BIG_MAX;
	}
	else
	{
		statusA = NORMOL_VALUE;
	}
	return statusA;
}
