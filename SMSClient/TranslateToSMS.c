//#include "stdafx.h"
#include "TranslateToSMS.h"
#include "MySQLOperation.h"
#include "global.h"
#include "cJSON.h"
#include "mysql.h"
#include "SendSMS.h"
#include "ATCMD.h"

void parseAlarmTable()
{
	char alarmContent[10240];
	memset(alarmContent, '\0', 10240);
	loadAlarmDictionary(alarmContent);
	alarmRoot_ = cJSON_Parse(alarmContent);
	if (!alarmRoot_) {
		WriteSystemLog(cJSON_GetErrorPtr());
		exit(1);
	}
}

DWORD WINAPI getAlarmData(LPVOID pParam)
{
	char authreq_data[1024];
	char order[10];   //命令
	char cpuid[20];
	int dataCount = 0;
	int dataLen = 0;
	char alarmContent[1024] = { 0 };
	bool isChars = false;
	char*parse_data = NULL;
	DataPacket * pdatapack=NULL;
	memset(order, '0', sizeof(order));
	memset(authreq_data, 0, sizeof(authreq_data));
	memset(cpuid, '\0', 20);
	do {
		Sleep(1);
		EnterCriticalSection(&g_cs);
		DATABUFFER*data_buffer = (DATABUFFER*)QUEUE_GetFirst(pPackageList);
		if (data_buffer == NULL) {
			printf("isnull\n");
			LeaveCriticalSection(&g_cs);
			break;
		}
		QUEUE_DelHead(pPackageList);
		LeaveCriticalSection(&g_cs);
		memcpy(authreq_data,data_buffer->data,1024);
		dataLen = data_buffer->size;
#ifdef DEBUG
		printf("告警数据：%s\n", authreq_data);
		for (int i = 0; i < dataLen; i++)
			printf("%02x ", authreq_data[i]);
		printf("\n");
		
#endif
		//如果是字节告警
		if (CheckByteCmd(authreq_data, dataLen) == 1) {
			PACKDEV packDev = *(PACKDEV*)authreq_data;
			unsigned char type = packDev.data[0];
			sprintf(order, "%d", type);
			sprintf(cpuid, "%d", packDev.devCpuId);
			parse_data = packDev.data;
			isChars = false;
#ifdef DEBUG
			printf("字节告警\n");
#endif 
		}
		//字符告警
		else {
			pdatapack = parse(authreq_data);
			parse_data = pdatapack->data;
			sprintf(order, "%s%s", pdatapack->morder, pdatapack->sorder);
			sprintf(cpuid, "%s", pdatapack->FH);
			isChars = true;
#ifdef DEBUG
	    	printf("字符告警\n");
#endif
		}

		//判断是否为需要转发的告警信息
		cJSON*itemList = NULL;
		itemList = cJSON_GetObjectItem(alarmRoot_, order);
		if (!itemList) 
		{
			printf("no list of %s\n", order);
		}
		else 
		{
			char devName[50] = { 0 };
			/*判断cpuid是否在管辖区域内*/
			if (getDistrictIndexAndName(cpuid,devName) == 0) {
				printf("%s: not exist!\n", cpuid);
			}
			else {
				memset(alarmContent, '\0', sizeof(alarmContent));
				sprintf(alarmContent, "%s", format);           //获取告警信息的格式
				replaceStr(alarmContent, "@device_name", devName);
				replaceStr(alarmContent, "@dev_gw_cpuid", cpuid);
				transAlarmData(parse_data, order, alarmContent,isChars);
			}
		}

		if (data_buffer != NULL) {
			memset(data_buffer, 0, sizeof(DATABUFFER));
			free(data_buffer);
			data_buffer = NULL;
		}
		if (pdatapack != NULL)free(pdatapack);

		EnterCriticalSection(&g_cs);
		dataCount = pPackageList->m_Count;
		printf("after: %d\n", dataCount);
		LeaveCriticalSection(&g_cs);
	} while (dataCount != 0);
	dataDeal_thread = FALSE;
	return 0;
}

void transAlarmData(const char*parse_data, const char*order, char*alarmContent,bool isChars) {
	cJSON*item = NULL;
	cJSON*orderItem= cJSON_GetObjectItem(alarmRoot_, order);   //获取告警命令对象
	char content[1024] = { 0 };   //告警内容
	char type[10] = {0};
	item = cJSON_GetObjectItem(orderItem, "type");
	sprintf(type, "%s", item->valuestring);
	//根据类型的不同做不同处理
	if (strcmp(type, "string")==0) {
		//字符串处理
		transStringAlarmData(parse_data, order, content);
	}
	else {
		//字节处理
		if (isChars) {
			char data[1024] = { 0 };
			parseStrToChars(parse_data, data);
			transByteAlarmData(data, order, content);
		}
		else {
			transByteAlarmData(parse_data, order, content);
		}
	}
	replaceStr(alarmContent, "@alarm_content", content);
	//判断以哪种方式发送SMS
	if (strcmp(sendWay, "cloud") == 0) {
		sendSMSByCloud(alarmContent);
	}
	else if (strcmp(sendWay, "DTU") == 0) {
		//sendSMSByDTU(alarmContent);
			printf("%s\n", alarmContent);
	}
}

void transStringAlarmData(const char*parse_data, const char*order, char*alarmContent)
{
	cJSON*itemList = NULL;
	cJSON*value = NULL;
	itemList = cJSON_GetObjectItem(alarmRoot_, order);
	value = cJSON_GetObjectItem(itemList, parse_data);
	sprintf(alarmContent, "%s %s", alarmContent, value->valuestring);
}

void transByteAlarmData(const char*parse_data, const char*order, char*alarmContent) 
{
	cJSON*item = NULL;
	cJSON*orderItem = cJSON_GetObjectItem(alarmRoot_, order);   //获取告警命令对象
	item = cJSON_GetObjectItem(orderItem, "multiNodes");
	if (item) {
		transAlarmLoop(parse_data, orderItem, alarmContent);
	}
	else {
		//不是多个节点同时上传
		transAlarmFiled(parse_data, orderItem, alarmContent);
	}
}

void transByBit(const char byteData,cJSON*alarmTypeRoot,char*alarmContent) {

	cJSON*item = cJSON_GetObjectItem(alarmTypeRoot,"value");
	int value = atoi(item->valuestring);
	cJSON*itemList = cJSON_GetObjectItem(alarmTypeRoot, "meaning");
	int size = cJSON_GetArraySize(itemList);
	int curValue = 0;
	if (size > 8) {
		printf("alarm table error");
		return;
	}
	for (int i=0; i < size; i++) {
		curValue = (byteData >> i) & 0x01;
		if (curValue == value) {
			item = cJSON_GetArrayItem(itemList, i);
			sprintf(alarmContent, "%s %s", alarmContent, item->valuestring);
		}
	}
}

void transByByte(const unsigned char byteData, cJSON*alarmTypeRoot, char*alarmContent) {
	int dataValue = byteData;
	cJSON*item = cJSON_GetObjectItem(alarmTypeRoot, "value");
	int size = cJSON_GetArraySize(item);
	cJSON*valueItem = NULL;
	if (strcmp(item->valuestring, "none") == 0) {
		item = cJSON_GetObjectItem(alarmTypeRoot, "meaning");
		sprintf(alarmContent, "%s %s:%d ", alarmContent, item->valuestring, dataValue);
	}
	else {
		for (int i = 0; i < size; i++) {
			valueItem = cJSON_GetArrayItem(item, i);
			int value = atoi(valueItem->valuestring);
			if (dataValue == value) {
				item = cJSON_GetObjectItem(alarmTypeRoot, "meaning");
				sprintf(alarmContent, "%s %s", alarmContent, item->valuestring);
			}
		}
	}
}

void transAlarmFiled(const char*parse_data, cJSON*orderItem, char*alarmContent) {
	char filed[10] = { 0 };
	unsigned char value = 0;
	cJSON*filedItem = NULL;
	cJSON*unitItem = NULL;
	cJSON*meanItem = cJSON_GetObjectItem(orderItem, "byteContent");
	cJSON*item = cJSON_GetObjectItem(orderItem, "alarmByte");
	int size = cJSON_GetArraySize(item);
	for (int i = 0; i < size; i++) {
		sprintf(filed, "%s", cJSON_GetArrayItem(item, i)->valuestring);
		filedItem = cJSON_GetObjectItem(meanItem, filed);
		unitItem = cJSON_GetObjectItem(filedItem, "unit");
		value = atoi(filed)&0x000000ff;
		value = parse_data[value];
		if (strcmp(unitItem->valuestring, "字节") == 0) {
			transByByte(value, filedItem, alarmContent);
		}
		else {
			transByBit(value, filedItem, alarmContent);
		}
	}
}

void transAlarmLoop(const char*parse_data, cJSON*orderItem, char*alarmContent) {
	short  count = 0;
	unsigned char value = 0;
	int size = 0;
	char filed[10] = { 0 };
	cJSON*filedItem = NULL;
	cJSON*unitItem = NULL;
	cJSON*item = NULL;
	item = cJSON_GetObjectItem(orderItem, "protocol");
	cJSON*meanItem = cJSON_GetObjectItem(orderItem, "protocol");
	if (strcmp(item->valuestring, "char")==0) {
		item = cJSON_GetObjectItem(orderItem, "nodesNumByte");
		count = atoi(item->valuestring);
		count = parse_data[count];
		item = cJSON_GetObjectItem(orderItem, "alarmByte");
		size = cJSON_GetArraySize(item);
	}
	else {
		item = cJSON_GetObjectItem(orderItem, "alarmByte");
		size = cJSON_GetArraySize(item);
		//count = (parse_data[1] << 8) + parse_data[2];
		char lenstr[10] = { 0 };
		sprintf(lenstr, "%s", cJSON_GetObjectItem(orderItem, "nodeLen")->valuestring);
		int len = atoi(lenstr);
		memcpy(&count, &parse_data[1], 2);
		count = count / len;
	}

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < size; j++) {
			sprintf(filed, "%s", cJSON_GetArrayItem(item, j)->valuestring);
			filedItem = cJSON_GetObjectItem(meanItem, filed);
			unitItem = cJSON_GetObjectItem(filedItem, "unit");
			value = atoi(filed) & 0x000000ff;
			value = parse_data[value+i*size];
			if (strcmp(unitItem->valuestring, "byte") == 0) {
				transByByte(value, filedItem, alarmContent);
			}
			else {
				transByBit(value, filedItem, alarmContent);
			}
		}
	}
}