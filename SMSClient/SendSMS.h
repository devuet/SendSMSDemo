#pragma once
#ifndef SENDSMS_H
#define SENDSMS_H
#include "sms_api.h"
#include "cJSON.h"
#include <stdio.h>
#include <Windows.h>
#include "global.h"
//DTU
extern DCB m_dcb;
extern HANDLE m_hComm;
//cloud
extern sms_send_message_request_2_t request;
extern sms_send_message_response_2_t resp;

cJSON*mapRoot_;
extern DISTRICTLIST districtList[MAXDESTRICTNUM];

void parseTelNumMap();

void initSerialPort();

void initRequest();

void sendSMSByCloud(char*alarmConten,int destrictIndex);

void sendSMSByDTU(char*alarmContent,int destrictIndex);


#endif // !SENDSMS_H
