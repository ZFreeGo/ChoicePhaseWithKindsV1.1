/***************************************************************
*Copyright(c) 2013, Sojo
*保留所有权利
*文件名称:BasicModule.h
*文件标识:
*创建日期： 2013年3月18日
*摘要: 用于基本模块
*当前版本:1.0
*作者: ZFREE
*取代版本:
*作者:
*完成时间:
************************************************************/

#ifndef BASICMODULE_H_
#define BASICMODULE_H_
#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "F2806x_Examples.h"   // F2806x Examples Include File

#ifdef	__cplusplus
extern "C" {
#endif

extern void DelayMs(Uint16 ms);  //MS 延时
extern void DelayUs(Uint16 us);  //us 延时

#ifdef	__cplusplus
}
#endif


#endif /* BASICMODULE_H_ */
