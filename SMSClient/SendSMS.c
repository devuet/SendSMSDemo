#include "SendSMS.h"
#include "global.h"

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
	m_hComm = CreateFile(
		"COM4:",
		GENERIC_READ | GENERIC_WRITE,   //允许读和写  
		0,                          //独占方式（共享模式）  
		NULL,
		OPEN_EXISTING,              //打开而不是创建（创建方式）  
		0,
		NULL
	);
	if (m_hComm == (HANDLE)-1)                          //打开串口失败返回  
	{
		WriteSystemLog("打开串口失败");
		exit(1);
	}
	//得到打开串口的当前属性参数，修改后再重新设置串口。  
	if (!GetCommState(m_hComm, &m_dcb))
	{
		WriteSystemLog("GetCommState error");
		return FALSE;
	}

	//设置串口参数  
	m_dcb.BaudRate = CBR_9600;   // 设置波特率9600  
	m_dcb.fBinary = TRUE; // 设置二进制模式，此处必须设置TRUE  
	m_dcb.fParity = TRUE; // 支持奇偶校验  
	m_dcb.fOutxCtsFlow = FALSE;  // No CTS output flow control  
	m_dcb.fOutxDsrFlow = FALSE;  // No DSR output flow control  
	m_dcb.fDtrControl = DTR_CONTROL_DISABLE; // No DTR flow control  
	m_dcb.fDsrSensitivity = FALSE; // DSR sensitivity  
	m_dcb.fTXContinueOnXoff = TRUE; // XOFF continues Tx  
	m_dcb.fOutX = FALSE;     // No XON/XOFF out flow control  
	m_dcb.fInX = FALSE;        // No XON/XOFF in flow control  
	m_dcb.fErrorChar = FALSE;    // Disable error replacement  
	m_dcb.fNull = FALSE;  // Disable null stripping  
	m_dcb.fRtsControl = RTS_CONTROL_DISABLE;   //No RTS flow control  
	m_dcb.fAbortOnError = FALSE;  // 当串口发生错误，并不终止串口读写  
	m_dcb.ByteSize = 8;   // 数据位,范围:4-8  
	m_dcb.Parity = NOPARITY; // 校验模式  
	m_dcb.StopBits = 0;   // 1位停止位  
						  //设置串口参数  
	if (!SetCommState(m_hComm, &m_dcb))
	{
		printf("SetCommState error");
		return FALSE;
	}
	SetupComm(m_hComm, 1024, 1024);//设置串口的输入/输出缓冲区大小
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);
	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(m_hComm, &TimeOuts); //设置超时 !mportant
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


void sendSMSByCloud(char*alarmContent,int destrictIndex)
{
	int size = districtList[destrictIndex].telCount;
	
	for (int i = 0; i < size; i++) {
		char*telNum; 
		telNum = districtList[destrictIndex].telephones[i];              //电话号码
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

void sendSMSByDTU(char*alarmContent, int destrictIndex)
{
	DWORD dwactlen;
	int size = districtList[destrictIndex].telCount;
	for (int i = 0; i < size; i++) {
		char*telNum;
		telNum = districtList[destrictIndex].telephones[i];              //电话号码
		WriteFile(m_hComm, alarmContent, MAXSIZE, &dwactlen, NULL);
	}
}
