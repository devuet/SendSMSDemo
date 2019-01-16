#pragma once
#ifndef SENDSMS_H
#define SENDSMS_H

#include <stdio.h>
#include <Windows.h>

#include "sms_api.h"
#include "cJSON.h"
#include "global.h"
//cloud
sms_send_message_request_2_t request;
sms_send_message_response_2_t resp;

cJSON*mapRoot_;
extern DISTRICTLIST districtList[MAXDESTRICTNUM];
extern int districtListCount;
extern int districtArray[MAXDESTRICTNUM];  //存放网关号所对应哪些区域
extern int districtArrayCount;

void parseTelNumMap();

void initSerialPort();

void initRequest();

void sendSMSByCloud(char*alarmConten);

void sendSMSByDTU(char*alarmContent);


#endif // !SENDSMS_H
