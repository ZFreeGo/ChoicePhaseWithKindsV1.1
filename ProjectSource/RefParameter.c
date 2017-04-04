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


void RefParameterInit(void)
{
	//A��
	g_ProcessDelayTime[PHASE_A].actionDelay = 0;
	g_ProcessDelayTime[PHASE_A].compensationTime = 0;
	g_ProcessDelayTime[PHASE_A].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_A].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_A].transmitDelay = 0;
	//B��
	g_ProcessDelayTime[PHASE_B].actionDelay = 0;
	g_ProcessDelayTime[PHASE_B].compensationTime = 0;
	g_ProcessDelayTime[PHASE_B].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_B].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_B].transmitDelay = 0;
	//C��
	g_ProcessDelayTime[PHASE_C].actionDelay = 0;
	g_ProcessDelayTime[PHASE_C].compensationTime = 0;
	g_ProcessDelayTime[PHASE_C].innerDelay =0 ;
	g_ProcessDelayTime[PHASE_C].sampleDelay = 0;
	g_ProcessDelayTime[PHASE_C].transmitDelay = 0;

	//У׼������ʼ��
	g_SystemCalibrationCoefficient.frequencyCoefficient = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient1 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient2 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient3 = 1;
	g_SystemCalibrationCoefficient.voltageCoefficient4 = 1;


	//ϵͳ��ѹ����
	 g_SystemVoltageParameter.VoltageC = 0;
	 g_SystemVoltageParameter.delayAB = 0;
	 g_SystemVoltageParameter.delayAC = 0;
	 g_SystemVoltageParameter.frequency = 50.0;
	 g_SystemVoltageParameter.period = 20000;
	 g_SystemVoltageParameter.voltage0 = 0;
	 g_SystemVoltageParameter.voltageA = 0;
	 g_SystemVoltageParameter.voltageB = 0;

	 //����Ԥ�Ʋ���
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


