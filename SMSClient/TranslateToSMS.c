//#include "stdafx.h"
#include "TranslateToSMS.h"
#include "MySQLOperation.h"
#include "global.h"
#include "cJSON.h"
#include "mysql.h"
#include "SendSMS.h"
#include "ATCMD.h"

bool isChars_ = false;   //�ж����ֽ�Э�黹���ַ�Э��

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
	char order[10];   //����
	char cpuid[20];
	int dataCount = 0;
	int dataLen = 0;
	char alarmContent[1024] = { 0 };
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
		printf("�澯���ݣ�%s\n", authreq_data);
		for (int i = 0; i < dataLen; i++)
			printf("%02x ", authreq_data[i]);
		printf("\n");
		
#endif
		//������ֽڸ澯
		if (CheckByteCmd(authreq_data, dataLen) == 1) {
			PACKDEV packDev = *(PACKDEV*)authreq_data;
			unsigned char type = packDev.data[0];
			sprintf(order, "%d", type);
			sprintf(cpuid, "%d", packDev.devCpuId);
			parse_data = packDev.data;
			isChars_ = false;
#ifdef DEBUG
			printf("�ֽڸ澯\n");
#endif 
		}
		//�ַ��澯
		else {
			pdatapack = parse(authreq_data);
			parse_data = pdatapack->data;
			sprintf(order, "%s%s", pdatapack->morder, pdatapack->sorder);
			sprintf(cpuid, "%s", pdatapack->FH);
			isChars_ = true;
#ifdef DEBUG
	    	printf("�ַ��澯\n");
#endif
		}

		//�ж��Ƿ�Ϊ��Ҫת���ĸ澯��Ϣ
		cJSON*itemList = NULL;
		itemList = cJSON_GetObjectItem(alarmRoot_, order);
		if (!itemList) 
		{
			printf("no list of %s\n", order);
		}
		else 
		{
			char devName[50] = { 0 };
			/*�ж�cpuid�Ƿ��ڹ�Ͻ������*/
			if (getDistrictIndexAndName(cpuid,devName) == 0) {
				printf("%s: not exist!\n", cpuid);
			}
			else {
				memset(alarmContent, '\0', sizeof(alarmContent));
				sprintf(alarmContent, "%s", format);           //��ȡ�澯��Ϣ�ĸ�ʽ
				replaceStr(alarmContent, "@device_name", devName);
				replaceStr(alarmContent, "@dev_gw_cpuid", cpuid);
				transAlarmData(parse_data, order, alarmContent);
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

void transAlarmData(const char*parse_data, const char*order, char*alarmContent) {
	cJSON*item = NULL;
	cJSON*orderItem= cJSON_GetObjectItem(alarmRoot_, order);   //��ȡ�澯�������
	char content[1024] = { 0 };   //�澯����
	char type[10] = {0};
	item = cJSON_GetObjectItem(orderItem, "type");
	sprintf(type, "%s", item->valuestring);
	//�������͵Ĳ�ͬ����ͬ����
	if (strcmp(type, "string")==0) {
		//�ַ�������
		transStringAlarmData(parse_data, order, content);
	}
	else {
		transByteAlarmData(parse_data, order, content);
	}
	replaceStr(alarmContent, "@alarm_content", content);
	//�ж������ַ�ʽ����SMS
	if (strcmp(sendWay, "cloud") == 0) {
		sendSMSByCloud(alarmContent);
	}
	else if (strcmp(sendWay, "DTU") == 0) {
		sendSMSByDTU(alarmContent);
	//		printf("%s\n", alarmContent);
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
	cJSON*orderItem = cJSON_GetObjectItem(alarmRoot_, order);   //��ȡ�澯�������
	item = cJSON_GetObjectItem(orderItem, "multiNodes");
	if (item && strcmp(item->valuestring,"yes")==0) {
		transAlarmLoop(parse_data, orderItem, alarmContent);
	}
	else {
		//���Ƕ���ڵ�ͬʱ�ϴ�
		transAlarmFiled(parse_data, orderItem, alarmContent);
	}
}

void transByBit(const int byteData,cJSON*alarmTypeRoot,char*alarmContent) {

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

void transByByte(const int byteData, cJSON*alarmTypeRoot, char*alarmContent) {
	cJSON*item = cJSON_GetObjectItem(alarmTypeRoot, "value");
	int size = cJSON_GetArraySize(item);
	cJSON*valueItem = NULL;
	char nodeAddr[10] = { 0 };
	valueItem = cJSON_GetArrayItem(item, 0);
	sprintf(nodeAddr, "%s",valueItem->valuestring);
	if ((1==size)&&(strcmp(nodeAddr, "-1")) == 0) {
		item = cJSON_GetObjectItem(alarmTypeRoot, "meaning");
		sprintf(alarmContent, "%s %s:%d ", alarmContent, item->valuestring, byteData);
	}
	else {
		for (int i = 0; i < size; i++) {
			valueItem = cJSON_GetArrayItem(item, i);
			int value = atoi(valueItem->valuestring);
			if (byteData == value) {
				valueItem = cJSON_GetObjectItem(alarmTypeRoot, "meaning");
				sprintf(alarmContent, "%s %s", alarmContent, valueItem->valuestring);
			}
		}
	}
}

void transAlarmFiled(const char*parse_data, cJSON*orderItem, char*alarmContent) {
	char filed[10] = { 0 };
	int value = 0,site = 0;
	int data[512] = { 0 };
	cJSON*filedItem = NULL;
	cJSON*unitItem = NULL;
	cJSON*meanItem = cJSON_GetObjectItem(orderItem, "byteContent");
	cJSON*item = cJSON_GetObjectItem(orderItem, "alarmByte");
	int size = cJSON_GetArraySize(item);   //�õ��澯�ֶ�����

	if (isChars_) {    //������ַ�Э�飬�������ݲ���ת����int��
		parseStrToChars(parse_data, data);
	}

	for (int i = 0; i < size; i++) {
		sprintf(filed, "%s", cJSON_GetArrayItem(item, i)->valuestring);
		filedItem = cJSON_GetObjectItem(meanItem, filed);
		unitItem = cJSON_GetObjectItem(filedItem, "unit");
		site = atoi(filed)&0x000000ff;
		if (isChars_) {    //������ַ�Э��
			value = data[site];
		}
		else {           //�ֽ�Э��
			value = (unsigned char)parse_data[site];    //תΪ�޷�����
		}
		if (strcmp(unitItem->valuestring, "byte") == 0) {  
			transByByte(value, filedItem, alarmContent);
		}
		else if(strcmp(unitItem->valuestring,"bit")==0){
			transByBit(value, filedItem, alarmContent);
		}
		else if (strcmp(unitItem->valuestring, "ADSTB")==0) {
			int addr = 0;
			if (isChars_) {
				addr = data[site];
			}
			else {
				unsigned char charTmp[2] = { 0 };
				charTmp[0] = parse_data[site]; charTmp[1] = parse_data[site + 1];
				addr = ((charTmp[0] << 8) + charTmp[1])&(0x3ff);
			}
			unitItem = cJSON_GetObjectItem(filedItem, "meaning");
			sprintf(alarmContent,"%s %s:%d", alarmContent,unitItem->valuestring, addr);
		}
	}
}

void transAlarmLoop(const char*parse_data, cJSON*orderItem, char*alarmContent) {
	short  count = 0;
	int value = 0,site = 0;
	int size = 0;
	int nodeLen = 0;
	char filed[10] = { 0 };
	int data[512] = { 0 };
	cJSON*filedItem = NULL;
	cJSON*unitItem = NULL;
	cJSON*item = NULL;
	item = cJSON_GetObjectItem(orderItem, "protocol");
	cJSON*meanItem = cJSON_GetObjectItem(orderItem, "byteContent");
	if (strcmp(item->valuestring, "char")==0) {
		parseStrToChars(parse_data, data);  //�����ַ������ݣ�ת��Ϊint��
		item = cJSON_GetObjectItem(orderItem, "nodesNumByte");
		count = atoi(item->valuestring);
		count = data[count];
		item = cJSON_GetObjectItem(orderItem, "alarmByte");
		size = cJSON_GetArraySize(item);
		char lenstr[10] = { 0 };
		sprintf(lenstr, "%s", cJSON_GetObjectItem(orderItem, "nodeLen")->valuestring);
		nodeLen = atoi(lenstr);
	}
	else {
		item = cJSON_GetObjectItem(orderItem, "alarmByte");
		size = cJSON_GetArraySize(item);
		//count = (parse_data[1] << 8) + parse_data[2];
		char lenstr[10] = { 0 };
		sprintf(lenstr, "%s", cJSON_GetObjectItem(orderItem, "nodeLen")->valuestring);
		nodeLen = atoi(lenstr);
		memcpy(&count, &parse_data[1], 2);
		count = count / nodeLen;
	}

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < size; j++) {
			sprintf(filed, "%s", cJSON_GetArrayItem(item, j)->valuestring);
			filedItem = cJSON_GetObjectItem(meanItem, filed);
			unitItem = cJSON_GetObjectItem(filedItem, "unit");
			site = atoi(filed) & 0x000000ff;
			site = site + i*nodeLen;
			if (isChars_) {    //������ַ�Э��
				value = data[site];
			}
			else {           //�ֽ�Э��
				value = (unsigned char)parse_data[site];    //תΪ�޷�����
			}
			if (strcmp(unitItem->valuestring, "byte") == 0) {
				transByByte(value, filedItem, alarmContent);
			}
			else if (strcmp(unitItem->valuestring, "bit") == 0) {
				transByBit(value, filedItem, alarmContent);
			}
			else if (strcmp(unitItem->valuestring, "ADSTB")==0) {  //���Ƶ�ַ��
				int addr = 0;
				if (isChars_) {
					addr = data[site];
				}
				else {
					unsigned char charTmp[2] = { 0 };
					charTmp[0] = parse_data[site]; charTmp[1] = parse_data[site + 1];
					addr = ((charTmp[0] << 8) + charTmp[1])&(0x3ff);
				}
				unitItem = cJSON_GetObjectItem(filedItem, "meaning");
				sprintf(alarmContent,"%s %s:%d", alarmContent,unitItem->valuestring, addr);
			}
		}
	}
}