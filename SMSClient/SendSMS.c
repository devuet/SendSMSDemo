#include "SendSMS.h"
#include "global.h"
#include "ATCMD.h"

void parseTelNumMap()
{
	char mapContent[1024];
	memset(mapContent, '\0', 1024);
	loadAlarmDictionary(mapContent);
	mapRoot_ = cJSON_Parse(mapContent);
	if (!mapRoot_) {
		WriteSystemLog(cJSON_GetErrorPtr());
		exit(1);
	}
}

void initSerialPort()
{
	//char SendData[1024] = { 0 };
	//char RecvData[1024] = { 0 };
	//打开COM口
	AT_ComOpen(); 
}
void initRequest()
{
	const char* ak = "963d07ecfe484299b0107e07********";  //百度云Access Key ID，从管理控制台中获取
	const char* sk = "4a9a8f4d7bbb4ebb8d15260c********";  //百度云Secret Access Key，从管理控制台中获取
	const char* host = "http://sms.bj.baidubce.com";      //百度云短信服务地址

	sms_config(ak, sk, host);

	request = sms_send_message_request_2_alloc();

	if (!request) {
		WriteSystemLog("allocate request object failed: %s\r\n", sms_get_last_error());
		exit(1);
	}
	//getchar();
	sms_send_message_request_2_set_invokeid(request, "dkwL6mUT-7JNv-H5Z3");
//	sms_send_message_request_2_set_phone_number(request, "13800000000");
	sms_send_message_request_2_set_template(request, "smsTpl:6c96f891-e42a-4fc3-b615-a56b21d6d464");

}
void sendSMSByCloud(char*alarmContent)
{
	int districtIndex, size;
	for (int count = 0; count < districtArrayCount; count++) {
		districtIndex = districtArray[count];
		size = districtList[districtIndex].telCount;
		for (int i = 0; i < size; i++) {
			char*telNum;
			telNum = districtList[districtIndex].telephones[i];              //电话号码
			sms_send_message_request_2_set_phone_number(request, telNum);
			sms_send_message_request_2_set_content_var(request, alarmContent);
			resp = sms_send_message_2(request);
			if (!resp) {
				WriteSystemLog(sms_get_last_error());
				return;
			}
#ifdef DEBUG
			printf("this msgid:%s\r\n", sms_send_message_response_2_get_message_id);
#endif
		}
	}

}

void sendSMSByDTU(char*alarmContent)
{
	int len=0,destrictIndex=0,size=0;
	for (int count = 0; count < districtArrayCount; count++) {
		destrictIndex = districtArray[count];
		size = districtList[destrictIndex].telCount;
		for (int i = 0; i < size; i++) {
			char*telNum;
			telNum = districtList[destrictIndex].telephones[i];              //电话号码
			if (telNum == NULL)
				return;
			else {
				len = strlen(alarmContent);
				AT_SendSMS(telNum,alarmContent, len);
			//	printf("%s\n", alarmContent);
			//	Sleep(5000);
			}
		}
	}
}
