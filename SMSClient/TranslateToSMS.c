//#include "stdafx.h"
#include "TranslateToSMS.h"
#include "MySQLOperation.h"
#include "global.h"
#include "cJSON.h"
#include "mysql.h"
#include "SendSMS.h"

void parseAlarmTable()
{
	char alarmContent[2048];
	memset(alarmContent, '\0', 2048);
	loadAlarmDictionary(alarmContent);
	alarmRoot_ = cJSON_Parse(alarmContent);
	if (!alarmRoot_) {
		WriteSystemLog(cJSON_GetErrorPtr());
		exit(1);
	}
}

int getDistrictIndex(const char * cpuID)
{
	int i, j;
	for (i = 0; i < districtListCount; i++) {
		for (j = 0; j < districtList[i].devCount; j++) {
			if (strcmp(cpuID, districtList[i].dev_cpuid[j]) == 0) {
				return i;
			}
		}
	}
	return -1;
}

void getAlarmMessage(const char*recv_data, const int recv_len, char*alarmContent)
{
	char authreq_data[1024];
	char order[10];   //����
	char cpuid[20];
	int destrictIndex = -1;
	char*parse_data = NULL;
	memset(order, '0', sizeof(order));
	memset(authreq_data, 0, sizeof(authreq_data));

	memset(cpuid, '\0', 20);
	memcpy(authreq_data, recv_data, 1024);

	//������ֽڸ澯
	if (CheckByteCmd(recv_data, recv_len) == 1) {
		PACKDEV packDev = *(PACKDEV*)recv_data;
		int type = packDev.data[0];
		sprintf(order, "%d", type);
		sprintf(cpuid, "%d", ntohl(packDev.devCpuId));
		parse_data = packDev.data;
	}
	//�ַ��澯
	else {            
		DataPacket * pdatapack = parse(authreq_data);
		parse_data = pdatapack->data;
		sprintf(order, "%s%s", pdatapack->morder, pdatapack->sorder);
		sprintf(cpuid, "%s", pdatapack->FH);
	}
	sprintf(alarmContent, "�澯���أ�%s,�澯���ݣ�", cpuid);
	destrictIndex = getDistrictIndex(cpuid);     //�õ��澯���ص�����id
	if (destrictIndex != -1) {                   //����Ƿ����ڹ�Ͻ����
		handleAlarmData(parse_data, order, alarmContent, destrictIndex);
	}


}

void  parseAlarmByByte(cJSON*alarmTypeRoot, const char alarmByte, char*alarmContent)
{
	int i = 0;
	int value = 0;
	int size = cJSON_GetArraySize(alarmTypeRoot);
	cJSON*item = NULL;
	if (size > 8) {
		printf("alarm table error");
		exit(1);
	}
	for (; i < size; i++) {
		value = (alarmByte >> i) & 0x01;
		if (value == 1) {
			item = cJSON_GetArrayItem(alarmTypeRoot, i);
			sprintf(alarmContent, "%s %s", alarmContent, item->valuestring);
		}
	}
}

void getLampAlarm(const char * parse_data, const char*order, char*alarmContent)
{
	int i = 0;
	int alarmAddr = 0;
	char alarmUp, alarmLow;
	int data[512];

	//���ַ�����ת����������ʽ
	parseStrToInt(parse_data, data);
	int lampNum = data[0];

	cJSON*itemList = NULL;
	cJSON*upList, *lowList;
	if (lampNum <= 0)
		return NULL;
	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	upList = cJSON_GetObjectItem(itemList, "1");    //��һ���ֽڴ�����ֵ����
	lowList = cJSON_GetObjectItem(itemList, "2");   //�ڶ����ֽڴ�����ֵ����
	if (!itemList) {
		printf("no list of %s", order);
		return NULL;
	}
	for (; i < lampNum; i++) {
		alarmAddr = (data[i * 3 + 1] & 0x000000ff);
		alarmUp = (data[i * 3 + 2] & 0x000000ff);
		alarmLow = (data[i * 3 + 3] & 0x000000ff);
		sprintf(alarmContent, "%s ���Ƶ�ַ��:%d ", alarmContent, alarmAddr);
		//��ȡ���޸澯
		parseAlarmByByte(upList, alarmUp, alarmContent);

		//��ȡ���޸澯
		parseAlarmByByte(lowList, alarmLow, alarmContent);

	}
	return alarmContent;
}

void getLoopAlarm(const char * parse_data, const char * order, char * alarmContent)
{
	short len = 0;
	cJSON*itemList = NULL;
	len = (parse_data[1] << 8) + parse_data[2];
	if (len != 2) {
		return;
	}
	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	char byte = parse_data[4];
	parseAlarmByByte(itemList, byte, alarmContent);
}

void getOtherAlarm(const char*parse_data, const char*order, char*alarmContent)
{
	cJSON*itemList = NULL;
	cJSON*value = NULL;

	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	if (!itemList) {
		printf("no list of %s", order);
		return NULL;
	}
	value = cJSON_GetObjectItem(itemList, parse_data);
	sprintf(alarmContent, "%s %s", alarmContent, value->valuestring);

	return alarmContent;
}

void getSunAlarm(const char * parse_data, const char * order, char * alarmContent)
{
	int data[512];
	cJSON*itemList,*value;
	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	parseStrToInt(parse_data, data);   //ת��int������
	uint8_t u8CommStat = data[4];
	uint8_t u8CurBatteryState = data[16];
	uint8_t u8CurLoadState = data[19];
	uint8_t u8OptState = data[21];

	//�жϸ澯״̬д�澯�����ݿ�
	char sql[1024] = { 0 };

	//�ж�����״̬��¼�澯
	if (0x02 != u8CurBatteryState)	//��������״̬����Ϊ�쳣
	{
		//��������״̬�쳣�澯
		value = cJSON_GetArrayItem(itemList, 1);
		sprintf(alarmContent, "%s%s", alarmContent, value->valuestring);
	}

	//�ж�̫���ܰ�״̬��¼�澯


	if (0x02 != u8OptState)
	{

		//����̫����״̬�澯
		value = cJSON_GetArrayItem(itemList, 0);
		sprintf(alarmContent, "%s%s", alarmContent, value->valuestring);

	}


	//�жϸ���״̬��¼�澯


	if (0x00 != u8CurLoadState && 0x01 != u8CurLoadState)	//���ǿ���״̬ʱ��Ϊ����״̬
	{

		//����״̬�澯
		value = cJSON_GetArrayItem(itemList, 3);
		sprintf(alarmContent, "%s%s", alarmContent, value->valuestring);

	}


	//�ж�ͨѶ��(RS485)״̬��¼�澯


	if (0x00 == u8CommStat)	//0x00��ʾ�����쳣
	{
		//������ͨѶ�쳣
		value = cJSON_GetArrayItem(itemList, 2);
		sprintf(alarmContent, "%s%s", alarmContent, value->valuestring);
	}

}

void handleAlarmData(const char * recv_data, const char * order, char * alarmContent, int destrictIndex)
{
	cJSON*itemList = NULL;
	char*alarm = NULL;
	//�ж��Ƿ�Ϊ��Ҫת���ĸ澯��Ϣ
	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	if (!itemList) {
		printf("no list of %s", order);
		return;
	}

	if (strcmp(order, "73") == 0) {
		getLampAlarm(recv_data, order, alarmContent);

	}
	if (strcmp(order, "15") == 0) {
		getLoopAlarm(recv_data, order, alarmContent);

	}
	if (strcmp(order, "719") == 0) {
		getOtherAlarm(recv_data, order, alarmContent);
	}
	if (strcmp(order, "74") == 0) {
		getSunAlarm(recv_data, order, alarmContent);
	}

	//�ж������ַ�ʽ����SMS
	if (strcmp(sendWay, "cloud") == 0) {
		sendSMSByCloud(alarmContent, destrictIndex);
	}
	else {
		sendSMSByDTU(alarmContent, destrictIndex);
	}
	WriteSystemLog(alarmContent);
}

