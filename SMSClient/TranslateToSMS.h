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

void transAlarmData(const char*parse_data, const char*order, char*alarmContent);

void transStringAlarmData(const char*parse_data, const char*order, char*alarmContent);

void transByteAlarmData(const char*parse_data, const char*order, char*alarmContent);

void transByBit(const int byteData, cJSON*alarmTypeRoot, char*alarmContent);

void transByByte(const int byteData, cJSON*alarmTypeRoot, char*alarmContent);

void transAlarmFiled(const char*parse_data, cJSON*orderItem, char*alarmContent);

void transAlarmLoop(const char*parse_data, cJSON*orderItem, char*alarmContent);

#endif // !TRANSLATE_H
