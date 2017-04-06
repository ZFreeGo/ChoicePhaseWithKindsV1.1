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
	uint16_t sampleDelay; //采样延时时间 us
	uint16_t innerDelay; //内部计算延时 us
	float calDelay;    //内部计算得出的进行的延时 us
	uint16_t transmitDelay;//传输延时 us
	uint16_t actionDelay;//动作延时--合闸 us
	float sumDelay;   //总延时= sampleDelay + innerDelay + transmitDelay + actionDelay
	int16_t compensationTime;// 补偿时间 us


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
typedef struct TagUpDownValue
{
	float upper;//上限
	float down;//下限

}UpDownValue;

/**
 * 系统限定参数值
 */
typedef struct TagLimitValue
{
	UpDownValue frequency; //频率
	UpDownValue workVoltage; //工作电压
	UpDownValue inportVoltage; //二次输入电压

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
    float realRatio;//以周期为对应时间COS，如PI/6 为1/12
    float realTime; //realRatio * 周期

    float startTime;//进行同步采样计算的 开始时间

}ActionRad;
/**
 *同步命令
 */
typedef struct TagSyncCommand
{
	uint8_t commandByte[8]; //命令字节数组
	uint8_t configbyte; //配置字
	uint16_t actionRadA; //A   设定弧度归一化值r = M/65536 *2*PI
	uint16_t actionRadB; //B
	uint16_t actionRadC; //C
}SyncCommand;


/**
 * 配置数据
 */
typedef struct TagConfigData
{
    uint8_t ID; //ID号
    void* pData;//指向数据
    uint8_t type;//类型
    void (*fSetValue)(PointUint8*, struct TagConfigData* )  ;
    void (*fGetValue)(PointUint8*, struct TagConfigData* )  ;
}ConfigData;


/**
 * 供外部调用的全局变量
 */
extern RefProcessTime g_ProcessDelayTime[3];
extern SystemCalibrationCoefficient g_SystemCalibrationCoefficient;
extern SystemVoltageParameter g_SystemVoltageParameter;
extern ActionRad g_PhaseActionRad[3];

extern void RefParameterInit(void);
#endif /* PROJECTHEADER_REFPARAMETER_H_ */
