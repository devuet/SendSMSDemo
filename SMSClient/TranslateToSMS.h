#pragma once

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "cJSON.h"
#include <stdio.h>
#include "global.h"


cJSON*alarmRoot_;
extern char sendWay[20];
extern DISTRICTLIST districtList[MAXDESTRICTNUM];
extern int districtListCount;

//��ȡ�����ļ��еĸ澯��������JSON��ʽ
void parseAlarmTable();

int getDistrictIndex(const char*cpuID);

//�����������������ݽ�������Ӧ�ĸ澯��Ϣ
void getAlarmMessage(const char*recv_data,const int recv_len,char*alarmContent);

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
void handleAlarmData(const char*parse_data, const char*order, char*alarmContent,int destrictIndex);


#endif // !TRANSLATE_H
