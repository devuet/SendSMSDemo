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


//��ȡ�����ļ��еĸ澯��������JSON��ʽ
void parseAlarmTable();

//�����������������ݽ�������Ӧ�ĸ澯��Ϣ
DWORD WINAPI getAlarmData(LPVOID pParam);

//�Ե����ֽڽ��н������õ����Ƹ澯
void parseAlarmByByte(cJSON*alarmTypeRoot,const char alarmByte,char*alarmContent);
//�õ����Ƹ澯
void getLampAlarm(const char * parse_data, const char*order,char*alarmContent);
//�õ���·�澯
void getLoopAlarm(const char*parse_data, const char*order, char*alarmContent);
//����������ʽ�ĸ澯���ģ���ʽ��һ��
void getOtherAlarm(const char*parse_data, const char*order,char*alarmContent);
//̫���Ƹ澯
void getSunAlarm(const char*parse_data, const char*order, char*alarmContent);
//�Բ�ͬ���͵ĸ澯��Ϣ���д���
void handleAlarmData(const char*parse_data, const char*order, char*alarmContent);


#endif // !TRANSLATE_H
