/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:ConfigADC.h
*文件标识:
*创建日期： 2015年1月31日
*摘要:
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#ifndef __CONFIGADC_H_
#define __CONFIGADC_H_

void ConfigADC_Monitor(float priod);//配置监控采样ADC
void StartSample(void);
void StopSample(void);
void SetSamplePriod(float us);

#endif /* CONFIGADC_H_ */
