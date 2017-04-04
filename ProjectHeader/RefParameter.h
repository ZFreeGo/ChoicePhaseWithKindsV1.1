/*
 * RefParameter.h
 *
 *  Created on: 2017年4月4日
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_REFPARAMETER_H_
#define PROJECTHEADER_REFPARAMETER_H_

#include "StdType.h"


/**
 * 用于电压控制
 */
#define PHASE_A  0
#define PHASE_B  1
#define PHASE_C  2


/**
 * 过程延时时间定义
 */
typedef struct TagProcessDelayTime
{
	float sampleDelay; //采样延时时间 us
	float innerDelay; //内部延时 us
	float transmitDelay;//传输延时 us
	float actionDelay;//动作延时--合闸 us
	float compensationTime;// 补偿时间 us

}RefProcessTime;

/**
 * 校准系数
 */
typedef struct TagSystemCalibrationCoefficient
{
	float voltageCoefficient1;//默认用于A相电压校准系数
	float voltageCoefficient2;//默认用于B相电压校准系数
	float voltageCoefficient3;//默认用于C相电压校准系数
	float voltageCoefficient4;//默认用于备用电压校准系数

	float frequencyCoefficient;//频率校准系数

}SystemCalibrationCoefficient;
/**
 * 三相电压参数
 */
typedef struct TagSystemVoltageParameter
{
	float voltageA;//默认用于A相电压 二次值
	float voltageB;//默认用于B相电压
	float VoltageC;//默认用于C相电压
	float voltage0;//默认用于备用电压

	float frequency;//频率 hz
	float period;//周期 us
	float delayAB; //B相滞后A的延时 us
	float delayAC; //相滞后A的延时 us

}SystemVoltageParameter;
/**
 * 上下限值
 */
typedef struct TagLimitValue
{
	float upper;//上限
	float down;//下限

}LimitValue;

/**
 *动作相角与弧度归一化值
 */
typedef struct TagActionParameter
{
	uint8_t enable; //使能标志，0-禁止，非零使能
    uint8_t phase; //相A-1,B-2,C-3
    uint16_t  actionRad; //设定弧度归一化值r = M/65536 *2*PI
    float realRad;

}ActionRad;

/**
 * 供外部调用的全局变量
 */
extern RefProcessTime g_ProcessDelayTime[3];
extern SystemCalibrationCoefficient g_SystemCalibrationCoefficient;
extern SystemVoltageParameter g_SystemVoltageParameter;
extern ActionRad g_PhaseActionRad[3];


#endif /* PROJECTHEADER_REFPARAMETER_H_ */
