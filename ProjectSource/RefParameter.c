/****************************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:MonitorCalculate.c
*文件标识:
*创建日期： 2017年4月4日
*摘要:
*2017/4/4:用来存放计算使用的各种数据
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
*******************************************************/

#include "RefParameter.h"

/**
 * 获取有效位数
 */
#define GET_ENOB(type) ((uint8_t)type & 0x0F)
/**
* 字节组成个数
*/
#define GET_BYTE_NUM(type) ((uint8_t)(type>>4) & 0x0F)


/*
 * 局部函数定义
 */
static void SetValueFloat32(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueFloat32(PointUint8* pPoint, ConfigData* pConfig);

static void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
static void SetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
static void GetValueUint8(PointUint8* pPoint, ConfigData* pConfig);
static void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig);

/**
 * 用于三相控制的延时参数
 */
RefProcessTime g_ProcessDelayTime[3];

/**
 * 系统参数校准
 */
SystemCalibrationCoefficient g_SystemCalibrationCoefficient;


/**
 * 系统三相电压有关参数
 */
SystemVoltageParameter g_SystemVoltageParameter;

/**
 *三相动作同步参数设定弧度
 */
ActionRad g_PhaseActionRad[3];

/**
 *系统参数上下限
 */
LimitValue g_SystemLimit;

/**
 *本地MAC
 */
uint8_t g_LocalMac;

/**
 *CAN错误
 */
volatile uint32_t g_CANErrorStatus;



uint8_t  BufferData[10];//接收缓冲数据
struct DefFrameData  g_NetSendFrame; //发送帧处理



#define PARAMETER_LEN 29  //设置参数列表
#define READONLY_PARAMETER_LEN 15  //读取参数列表
#define READONLY_START_ID 0x41
/**
 *系统配置参数合集
 */
ConfigData g_SetParameterCollect[PARAMETER_LEN]; //配置参数列表--可读可写
ConfigData g_ReadOnlyParameterCollect[READONLY_PARAMETER_LEN]; //参数合集--只读列表




/**
 * 设置参数
 * @param id      配置号
 * @param pPoint  指向保存数据的指针
 *
 * @return 错误代码 0-没有错误 非0-有错误
 */
uint8_t SetParamValue(uint8_t id, PointUint8* pPoint)
{

	for(uint8_t i = 0; i < PARAMETER_LEN; i++)
	{
		if(g_SetParameterCollect[i].ID == id)
		{
			//TODO :添加错误处理——每一个Set/Get函数添加相应的处理内容。
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
 * 读取参数
 * @param id      配置号
 * @param pPoint  指向保存数据的指针
 *
 * @return 错误代码 0-没有错误 非0-有错误
 */
uint8_t ReadParamValue(uint8_t id, PointUint8* pPoint)
{
	if (id < READONLY_START_ID) //小于只读ID，可为配置参数类型
	{
		for(uint8_t i = 0; i < PARAMETER_LEN; i++)
		{
			if(g_SetParameterCollect[i].ID == id)
			{
				//TODO :添加错误处理——每一个Get函数添加相应的处理内容。

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
				//TODO :添加错误处理——每一个et函数添加相应的处理内容。

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
 * 初始化只读系统参数合集
 */
static void InitReadonlyParameterCollect(void)
{
	uint8_t index = 0, id = READONLY_START_ID;
	//Uint32 -- 二次电压值，保留6位小数
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
	//Uint16 --[0,65535]，单位us
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
 * 初始化系统参数合集
 */
static void InitSetParameterCollect(void)
{
	uint8_t index = 0, id = 1;
	//Uint32 -- 互感器输入到检测之间的校准系数，保留6位小数
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

	//Uint16 --[0-65535]，单位us
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
	//Uint16 --[0-65535]，单位ms
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
	//Uint16 归一化值 合闸相角
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

	if (PARAMETER_LEN < index)
	{
		while(1);
	}

}


/**
 * 初始化全局变量参数
 */
void RefParameterInit(void)
{
	//A相
	g_ProcessDelayTime[PHASE_A].actionDelay = 0;
	g_ProcessDelayTime[PHASE_A].compensationTime = 0;
	g_ProcessDelayTime[PHASE_A].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_A].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_A].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_A].sumDelay = 0;
	g_ProcessDelayTime[PHASE_A].calDelay = 0;
	//B相
	g_ProcessDelayTime[PHASE_B].actionDelay = 0;
	g_ProcessDelayTime[PHASE_B].compensationTime = 0;
	g_ProcessDelayTime[PHASE_B].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_B].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_B].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_B].sumDelay = 0;
	g_ProcessDelayTime[PHASE_B].calDelay = 0;
	//C相
	g_ProcessDelayTime[PHASE_C].actionDelay = 0;
	g_ProcessDelayTime[PHASE_C].compensationTime = 0;
	g_ProcessDelayTime[PHASE_C].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_C].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_C].transmitDelay = 0;
	g_ProcessDelayTime[PHASE_C].sumDelay = 0;
	g_ProcessDelayTime[PHASE_C].calDelay = 0;

	//校准参数初始化
	g_SystemCalibrationCoefficient.frequencyCoefficient = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient1 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient2 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient3 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient4 = 1;


	//系统电压参数
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

	 //三相预制参数 realRatio
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


	 //系统参数上下限
	 g_SystemLimit.frequency.max = 55.0f;
	 g_SystemLimit.frequency.min = 45.0f;
	 g_SystemLimit.workVoltage.max = 3.5f;
	 g_SystemLimit.workVoltage.min =  3.1f;
	 g_SystemLimit.inportVoltage.min = 80;
	 g_SystemLimit.inportVoltage.max = 250;
	 //同步预制等待时间
	 g_SystemLimit.syncReadyWaitTime = 3000;


	 //CAN通讯错误状态
	 g_CANErrorStatus = 0;


	 g_LocalMac = 0x0D;

	 InitSetParameterCollect();
	 InitReadonlyParameterCollect();

	 //缓冲数据发送
	 g_NetSendFrame.pBuffer = BufferData;


}


/**
 * 设置系统校准系数[0,4294.967295]，默认保留六位小数;
 * 适用于校准系数,二次电压值
 * <p>
 * 以float形式存储，以4字节进行交换传输的数据
 * 
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
static void SetValueFloat32(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >=  4)
	{
		//Todo: 根据ID选择保留有效位数
		float ration = 0.000001f;
	    uint32_t data1 = pPoint->pData[1];
	    uint32_t data2 = pPoint->pData[2];
	    uint32_t data3 = pPoint->pData[3];
	    uint32_t data = (data3 << 24) | (data2 << 16) | (data1 << 8)|pPoint->pData[0] ;

		float result = (float)data *  ration;
	    *(float*)pConfig->pData = result;

		//TODO:校准系数需考虑EEPROM存储
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
* 获取系统校准系数[0,4294.967295]，默认保留六位小数;
 * 适用于校准系数,二次电压值
 * <p>
 * 以float形式存储，以4字节进行交换传输的数据
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
static void GetValueFloat32(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 4)
	{
		//Todo: 根据ID选择保留有效位数

		float ration = 1000000.0f;
		if (pConfig->type == 0x42)
		{
			ration = 100.0f;
		}


		uint32_t result = (uint32_t)(*(float*)pConfig->pData * ration);
		pPoint->pData[0] = (uint8_t)(result & 0x00FF);
		pPoint->pData[1] = (uint8_t)(result >> 8);
		pPoint->pData[2] = (uint8_t)(result >> 16);
		pPoint->pData[3] = (uint8_t)(result >> 24);
		pPoint->len = 4;
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
	}
}


/**
 * 设置参数[0,65.535]，默认保留3位小数
 * 适用于电压频率,工作电压，
 * <p>
 * 以float存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
static void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		//Todo: 更具ID选择保留有效位数
		float ration = 0.001f;
		uint16_t data = pPoint->pData[1];
		data =  (data << 8) | pPoint->pData[0];
		float result = (float)data *  ration;
		*(float*)pConfig->pData = result;

		//TODO:需考虑EEPROM存储
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 获取参数[0,65.535]，默认保留3位小数
 * 适用于电压频率,工作电压，
 * <p>
 * 以float存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
static void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 4)
	{
		//Todo: 更具ID选择保留有效位数
		float ration = 1000.0f;

		uint16_t result = (uint16_t)(*(float*)pConfig->pData * ration);
		pPoint->pData[0] = (uint8_t)(result & 0x00FF);
		pPoint->pData[1] = (uint8_t)(result >> 8);
		pPoint->len = 2;
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 设置值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint16_t形式存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
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
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 获取值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint16_t形式存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
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
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 设置值，针对1字节[0,255]
 *
 * <p>
 * 以uint8_t形式存储，以1字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
static void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 1)
	{
		*(uint8_t*)pConfig->pData = pPoint->pData[0];
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 获取值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint8_t形式存储，以1字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
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
		pPoint->len = 0; //置为0，以示意错误
	}
}


