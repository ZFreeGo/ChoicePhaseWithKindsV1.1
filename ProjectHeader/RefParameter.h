/*
 * RefParameter.h
 *
 *  Created on: 2017年4月4日
 *      Author: ZYF
 */

#ifndef PROJECTHEADER_REFPARAMETER_H_
#define PROJECTHEADER_REFPARAMETER_H_

#include "StdType.h"
#include "DeviceNet.h"

//EEPROM地址
#define EEPROM_ADDRESS  0xA0

/**
 * 用于电压控制
 */
#define PHASE_A  0
#define PHASE_B  1
#define PHASE_C  2


#define WOKE_WATCH  0xA1
#define WOKE_CONFIG 0xAA
#define WOKE_NORMAL 0x55

#define PULSE_COUNT 10 //脉冲计数


#define CAL_FACTOR 0.2098083496094 //理论计算系数

//频率合集
typedef struct RagFreqCollect
{
	float FreqInit;//初始频频率设置
	float FreqReal;//当前实时频率
	float FreqMean;//新计算实时频率
	float FreqCal;
} FreqCollect;

/**
 * 过程延时时间定义
 */
typedef struct TagProcessDelayTime
{
	uint16_t sampleDelay; //采样延时时间 us
	uint16_t innerDelay; //内部计算延时 us
	uint16_t pulseDelay;  //同步脉冲序列延时us (400us)
	float calDelay;    //内部计算得出的进行的延时 us
	float calDelayCheck;    //校验后的内部计算得出的进行的延时 us
	uint16_t transmitDelay;//传输延时 us
	uint16_t actionDelay;//动作延时--合闸 us
	float sumDelay;   //总延时= sampleDelay + innerDelay + transmitDelay + actionDelay  + pulseDelay
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
	float voltageC;//默认用于C相电压
	float voltage0;//默认用于备用电压

	FreqCollect frequencyCollect;//频率 hz
	float period;//周期 us
	uint16_t delayAB; //B相滞后A的延时 us
	uint16_t delayAC; //相滞后A的延时 us

	float workVoltage; //工作电压



	float perodCap;//通过硬件捕捉计算实时周期
	float perodMeanCap;//通过硬件捕捉计算平均周期

}SystemVoltageParameter;
/**
 * 上下限值
 */
typedef struct TagMaxMinValue
{
	float max;//上限
	float min;//下限

}MaxMinValue;

/**
 * 系统限定参数值
 */
typedef struct TagLimitValue
{
	MaxMinValue frequency; //频率
	MaxMinValue workVoltage; //工作电压
	MaxMinValue inportVoltage; //二次输入电压
	uint16_t syncReadyWaitTime;

}LimitValue;


/**
 *动作相角与弧度归一化值
 */
typedef struct TagActionParameter
{
	uint8_t readyFlag; //相应控制器应答 准备标志
	uint8_t enable; //使能标志，0-禁止，非零使能
    uint8_t phase; //相A-1,B-2,C-3
    uint16_t  actionRad; //设定弧度归一化值r = M/65536 *2*PI
    float realRad; // 对应相对上一项角差
    float realDiffRatio;//差值比率，程序设定值.与前一项的差值，首相的前一项差值为0
    float realRatio;//以周期为对应时间COS，如PI/6 为1/12
    float realTime; //realRatio * 周期
    float realDiffTime; //realRatio * 周期

    float startTime;//进行同步采样计算的 开始时间
    uint8_t loopByte;//回路控制字
    uint8_t count; //控制数量

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
