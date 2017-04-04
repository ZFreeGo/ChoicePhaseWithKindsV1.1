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


void RefParameterInit(void)
{
	//A相
	g_ProcessDelayTime[PHASE_A].actionDelay = 0;
	g_ProcessDelayTime[PHASE_A].compensationTime = 0;
	g_ProcessDelayTime[PHASE_A].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_A].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_A].transmitDelay = 0;
	//B相
	g_ProcessDelayTime[PHASE_B].actionDelay = 0;
	g_ProcessDelayTime[PHASE_B].compensationTime = 0;
	g_ProcessDelayTime[PHASE_B].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_B].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_B].transmitDelay = 0;
	//C相
	g_ProcessDelayTime[PHASE_C].actionDelay = 0;
	g_ProcessDelayTime[PHASE_C].compensationTime = 0;
	g_ProcessDelayTime[PHASE_C].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_C].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_C].transmitDelay = 0;

	//校准参数初始化
	g_SystemCalibrationCoefficient.frequencyCoefficient = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient1 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient2 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient3 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient4 = 1;


	//系统电压参数
	 g_SystemVoltageParameter.VoltageC = 0;
	 g_SystemVoltageParameter.delayAB = 0;
	 g_SystemVoltageParameter.delayAC = 0;
	 g_SystemVoltageParameter.frequency = 50.0;
	 g_SystemVoltageParameter.period = 20000;
	 g_SystemVoltageParameter.voltage0 = 0;
	 g_SystemVoltageParameter.voltageA = 0;
	 g_SystemVoltageParameter.voltageB = 0;

	 //三相预制参数
	 g_PhaseActionRad[0].phase = PHASE_A;
	 g_PhaseActionRad[0].actionRad = 0;
	 g_PhaseActionRad[0].enable = 0xff;
	 g_PhaseActionRad[1].phase = PHASE_B;
	 g_PhaseActionRad[1].actionRad = 0;
	 g_PhaseActionRad[1].enable = 0xff;
	 g_PhaseActionRad[2].phase = PHASE_C;
	 g_PhaseActionRad[2].actionRad = 0;
	 g_PhaseActionRad[2].enable = 0xff;

}


