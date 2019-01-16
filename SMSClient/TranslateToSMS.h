#pragma once

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "cJSON.h"
#include <stdio.h>
#include "global.h"


cJSON*alarmRoot_;
extern char sendWay[20];
//extern DISTRICTLIST districtList[MAXDESTRICTNUM];
//extern int districtListCount;


//读取配置文件中的告警表并解析成JSON格式
void parseAlarmTable();

//将服务器发来的数据解析成相应的告警信息
DWORD WINAPI getAlarmData(LPVOID pParam);

//对单个字节进行解析，得到单灯告警
void parseAlarmByByte(cJSON*alarmTypeRoot,const char alarmByte,char*alarmContent);
//得到单灯告警
void getLampAlarm(const char * parse_data, const char*order,char*alarmContent);
//得到回路告警
void getLoopAlarm(const char*parse_data, const char*order, char*alarmContent);
//解析其他形式的告警信心，格式需一致
void getOtherAlarm(const char*parse_data, const char*order,char*alarmContent);
//太阳灯告警
void getSunAlarm(const char*parse_data, const char*order, char*alarmContent);
//对不同类型的告警信息进行处理
void handleAlarmData(const char*parse_data, const char*order, char*alarmContent);


#endif // !TRANSLATE_H
