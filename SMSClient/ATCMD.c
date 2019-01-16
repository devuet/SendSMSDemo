#include <stdio.h>
#include <string.h>
#include "ATCMD.h"
#include "global.h"
static HANDLE hCom;
static char com_name[32] = { 0 };
static int BaudRate;

int AT_ComOpen()

{
	BaudRate= CBR_115200;
	sprintf(com_name, "%s", "COM5");
	hCom = SerialOpen(com_name);
	if (!hCom)

	{

		printf("串口端口打开失败\n");

		return -1;

	}



	SerialSet(hCom, BaudRate);

	return 0;

}

int AT_ComClose()

{

	SerialClose(hCom);



	return 0;

}

int AT_SendCmd(char *cmd, int len)

{

	if (SerialSend(hCom, cmd, len) < 0)

	{

		printf("SendCmd Failed\n");

		return -1;

	}



	return 0;

}

int AT_RecvData(char *rdata)

{

	SerialRecv(hCom, rdata);


	return 0;

}

static int ConverToUnicode(char *indata, char *outdata)

{

	int len = strlen(indata);

	int wlen = sizeof(wchar_t)*len;

	int i = 0;

	char buf[1024];



	wchar_t *wch = (wchar_t *)malloc(wlen);



	memset(wch, 0, wlen);

	MultiByteToWideChar(CP_ACP, 0, indata, len, wch, wlen);



	memset(buf, 0, sizeof(buf));

	for (i = 0; i < len; i++)

	{

		if (wch[i] == 0)

			continue;

		sprintf(buf + strlen(buf), "%04x", wch[i]);

	}



	sprintf(outdata, "%02x", strlen(buf) / 2);

	strcat(outdata, buf);

	free(wch);



	return 0;

}

static int phonenum_parity_exchange(char *indata, char *outdata)

{

	int i = 0;

	int len = strlen(indata);

	char tmp[16];

	int ret = len % 2;



	memset(tmp, 0, sizeof(tmp));

	strcpy(tmp, indata);



	if (ret != 0) {

		strcat(tmp, "f");

		len = len + 1;

	}



	for (i = 0; i < len; i += 2)

	{

		outdata[i] = tmp[i + 1];

		outdata[i + 1] = tmp[i];

	}



	return 0;

}

static int get_sms_center(char *sm)

{

	char SendData[32];

	char rdata[64];

	char *num_start;

	char *num_end;

	int num_len;

	char tmp[16];



	sprintf(SendData, "%s\r", "AT+CSCA?");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		printf("set pdumode Failed\n");

		AT_ComClose();

		return -1;

	}



	memset(rdata, 0, sizeof(rdata));

	AT_RecvData(rdata);

	printf("rdata:%s\n", rdata);

	num_start = strchr(rdata, '"');

	if (num_start == NULL)

		return -1;

	else

		*num_start = '\0';



	printf("rdata num:%s\n", num_start + 1);

	num_end = strchr(num_start + 1, '"');

	if (num_end == NULL)

		return -1;

	else

		*num_end = '\0';



	printf("rdata start:%s\n", num_start + 2);

	memset(tmp, 0, sizeof(tmp));

	phonenum_parity_exchange(num_start + 2, tmp);



	num_len = (strlen(tmp) + 2) / 2;

	sprintf(sm, "%02x%s%s", num_len, "91", tmp);



	return 0;

}

static int get_sms_dest(char *phonenum, char *dest)

{

	char tmp[16];

	int num_len = strlen(phonenum);



	memset(tmp, 0, sizeof(tmp));

	if (strchr(phonenum, '+'))

	{

		num_len = num_len - 1;

		phonenum_parity_exchange(phonenum + 1, tmp);

#ifdef DEBUG
		sprintf(dest, "%s%02x%s", "1100", num_len, "91");
#endif

	}

	else

	{

		phonenum_parity_exchange(phonenum, tmp);

#ifdef DEBUG
		sprintf(dest, "%s%02x%s", "1100", num_len, "81");
#endif

	}



	strcat(dest, tmp);

	strcat(dest, "000800");



	return 0;

}

int AT_ComConfig(char *name, int rate)

{

	strcpy(com_name, name);

	BaudRate = rate;



	if (AT_ComOpen())

		return -1;



	AT_ComClose();



	return 0;

}

int AT_ApnConfig(char *apn)

{

	char SendData[512] = { 0 };

	int cid = 1;

	char *pdp_type = "ip";



	if (AT_ComOpen())

		return -1;



	/* apn config cmd:at+CGDCONT=[<cid> [,<PDP_type> [,<APN> [,<PDP_addr> [,<d_comp> [,<h_comp>]]]]]*/

	sprintf(SendData, "at+cgdcont=%d,\"%s\", \"%s\"", cid, pdp_type, apn);



	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		AT_ComClose();

		return -1;

	}



	AT_ComClose();

	return 0;

}

int SMS_TextMode(char *phonenum, char *msg)

{

	char SendData[512] = { 0 };

	char RecvData[1024] = { 0 };



	/* 设置短信发送格式为文本格式 */

	sprintf(SendData, "%s\r", "AT+CMGF=1");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		printf("set textmode Failed\n");

		AT_ComClose();

		return -1;

	}



//#if 0

	// for debug

	Sleep(2);

	SerialRecv(hCom, RecvData);

	printf("Recv data:%s\n", RecvData);

//#endif



	/* 设置短信发送号码 */

	sprintf(SendData, "AT+CMGS=\"%s\"\r", phonenum);

	if (SerialSend(hCom, SendData, strlen(SendData)) < 0)

	{

		printf("send phone number Failed\n");

		AT_ComClose();

		return -1;

	}



	/* 发送短信内容 */

	printf("send msg:%s\n", msg);

	sprintf(SendData, "%s", msg);

	strcat(SendData, "\x1a");

	if (SerialSend(hCom, SendData, strlen(SendData)) < 0)

	{

		printf("send msg Failed\n");

		AT_ComClose();

		return -1;

	}



	return 0;

}

int SMS_PduMode(char *phonenum,char *msg)

{

	char SendData[2048] = { 0 };

	char RecvData[1024] = { 0 };

	char systemLog[1024] = { 0 };

	char sms_dest[32];

	char pdumsg[1024];

	char sms_center[32];

	int len;

	sprintf(systemLog, "%s:%s", phonenum, msg);

	/*get message send address */

	memset(sms_dest, 0, sizeof(sms_dest));

	get_sms_dest(phonenum, sms_dest);

	/* convert gb2312 to unicode */

	ConverToUnicode(msg, pdumsg);

	len = (strlen(pdumsg) + strlen(sms_dest)) / 2;

	/* 设置短信发送格式为pdu格式 */

	sprintf(SendData, "%s\r", "AT+CMGF=0");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		WriteSystemLog("set pdumode Failed\n");

		//AT_ComClose();

		//exit(1);

	}

	memset(RecvData, 0, sizeof(RecvData));

	AT_RecvData(RecvData);

	if (!strstr(RecvData, "OK"))

	{

		WriteSystemLog("set pdumode Failed\n");

		//AT_ComClose();

		//exit(1);

	}

	/* get the service message center address */

	memset(sms_center, 0, sizeof(sms_center));

	if (get_sms_center(sms_center) != 0) 

	{

		WriteSystemLog("get sms_center faild!");

		return -1;
	}

#ifdef DEBUG

	printf("sms_center:%s\n", sms_center);

#endif
	/* 设置短信发送号码 */

	sprintf(SendData, "AT+CMGS=%d\r", len);

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		WriteSystemLog("send phone number Failed\n");

		//AT_ComClose();
		return -1;

	}

	/* 发送短信内容 */

	sprintf(SendData, "%s%s%s", sms_center, sms_dest, pdumsg);

	//printf("send msg:%s, len=%d\n", SendData, strlen(SendData));

	strcat(SendData, "\x1a");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		WriteSystemLog(msg);

		//AT_ComClose();

		return -1;

	}
	if (!strstr(RecvData, "OK"))

	{

		WriteSystemLog(msg);

	//	AT_ComClose();
		return -1;

	}
	
	WriteSystemLog(systemLog);

	Sleep(5000);  //延时5s，等GSM发送完信息

	return 0;

}

int AT_SendSMS(char *phonenum,char *msg, int msg_len)

{

	SMS_PduMode(phonenum, msg);

	return 0;

}

int AT_GetFlux(char *rdata, int buf_len)

{

	char SendData[32];

	char RecvData[512];

	char *ptr;



	*rdata = '\0';



	if (AT_ComOpen())

		return -1;



	/* 获取流量信息 */

	sprintf(SendData, "%s\r", "AT^DSFLOWQRY");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		printf("send get flux cmd Failed\n");

		AT_ComClose();

		return -1;

	}



	memset(RecvData, 0, sizeof(RecvData));

	AT_RecvData(RecvData);

	printf("get flux read:%s\n", RecvData);



	ptr = strchr(RecvData, ':');

	if (ptr == NULL)

	{

		AT_ComClose();

		return -1;

	}

	else

		strncpy(rdata, ptr + 1, buf_len);



	AT_ComClose();

	return 0;

}

int AT_GetCi(char *rdata, int buf_len)

{

	char SendData[32];

	char RecvData[64];

	char *ptr;



	*rdata = '\0';



	if (AT_ComOpen())

		return -1;



	Sleep(1);



	/* 设置允许获取位置信息 */

	sprintf(SendData, "%s\r", "AT+CREG=2");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		printf("set get cell id Failed\n");

		AT_ComClose();

		return -2;

	}



	memset(RecvData, 0, sizeof(RecvData));

	AT_RecvData(RecvData);

	if (!strstr(RecvData, "OK"))

	{

		AT_ComClose();

		return -3;

	}



	/* 获取位置信息 */

	sprintf(SendData, "%s\r", "AT+CREG?");

	if (AT_SendCmd(SendData, strlen(SendData)) < 0)

	{

		printf("Get cell id Failed\n");

		AT_ComClose();

		return -4;

	}





	AT_RecvData(RecvData);

	printf("getci read:%s\n", RecvData);



	ptr = strrchr(RecvData, ',');

	if (ptr == NULL)

	{

		AT_ComClose();

		return -5;

	}

	else

		strncpy(rdata, ptr + 1, buf_len);



	AT_ComClose();

	return 0;

}

HANDLE SerialOpen(char * name)
{
	HANDLE com;
	com = CreateFile(L"COM5",
		GENERIC_READ | GENERIC_WRITE, //允许读写
		0,//独占
		NULL,
		OPEN_EXISTING,//打开而不是创建
		0,//同步方式
		NULL);
	if (com == INVALID_HANDLE_VALUE)
	{
		WriteSystemLog("打开COM失败!");
		exit(1);
	}
	return com;
}

int SerialSet(HANDLE fd, int rate)
{
	DCB myDCB;
	if (!GetCommState(fd, &myDCB))
	{
		printf("GetCommState error");
		return FALSE;
	}


	//设置串口参数  
	myDCB.BaudRate = rate;   // 设置波特率115200  
	myDCB.fBinary = TRUE; // 设置二进制模式，此处必须设置TRUE  
	myDCB.fParity = TRUE; // 支持奇偶校验  
	myDCB.fOutxCtsFlow = FALSE;  // No CTS output flow control  
	myDCB.fOutxDsrFlow = FALSE;  // No DSR output flow control  
	myDCB.fDtrControl = DTR_CONTROL_DISABLE; // No DTR flow control  
	myDCB.fDsrSensitivity = FALSE; // DSR sensitivity  
	myDCB.fTXContinueOnXoff = TRUE; // XOFF continues Tx  
	myDCB.fOutX = FALSE;     // No XON/XOFF out flow control  
	myDCB.fInX = FALSE;        // No XON/XOFF in flow control  
	myDCB.fErrorChar = FALSE;    // Disable error replacement  
	myDCB.fNull = FALSE;  // Disable null stripping  
	myDCB.fRtsControl = RTS_CONTROL_DISABLE;   //No RTS flow control  
	myDCB.fAbortOnError = FALSE;  // 当串口发生错误，并不终止串口读写  
	myDCB.ByteSize = 8;   // 数据位,范围:4-8  
	myDCB.Parity = NOPARITY; // 校验模式  
	myDCB.StopBits = 0;   // 1位停止位  
						  //设置串口参数  
	if (!SetCommState(fd, &myDCB))
	{
		WriteSystemLog("SetCommState error");
		return FALSE;
	}
	SetupComm(fd, 1024, 1024);//设置串口的输入/输出缓冲区大小
	PurgeComm(fd, PURGE_RXCLEAR | PURGE_TXCLEAR);
	COMMTIMEOUTS TimeOuts;
	//设定读超时
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 1;
	TimeOuts.ReadTotalTimeoutConstant = 100;
	//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	TimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(fd, &TimeOuts); //设置超时 !mportant
}

int SerialSend(HANDLE fd, char * data, int len)
{
	Sleep(500);
	DWORD dwactlen;
	WriteFile(fd, data, len, &dwactlen, NULL);
	PurgeComm(fd, PURGE_RXCLEAR | PURGE_TXCLEAR);
	return dwactlen;
}

int SerialRecv(HANDLE fd, char * data)
{
	DWORD dwactlen;
	Sleep(100);
	ReadFile(fd, data, 1024, &dwactlen,NULL);
	PurgeComm(fd, PURGE_RXCLEAR | PURGE_TXCLEAR);
	return dwactlen;

}

int SerialClose(HANDLE fd)
{
	CloseHandle(fd);
}
