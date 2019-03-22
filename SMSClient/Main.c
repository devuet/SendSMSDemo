//#include "stdafx.h"
#include "global.h"
#include "TranslateToSMS.h"
#include "SendSMS.h"
#include "MySQLOperation.h"
#include "ATCMD.h"
#include "queue.h"
//#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

char sendWay[20];
char format[512];
tQUEUE*pPackageList;
CRITICAL_SECTION g_cs;
BOOL dataDeal_thread = FALSE;
int main()
{
	WSADATA WSAData;
	struct sockaddr serverAddr;
	struct sockaddr localAddr;
	struct sockaddr_in *sin;
	fd_set			readfds;
	int serverSockfd;
	char recv_buffer[1024];
	char temp_buffer[1024];
	int serverPort= 19732;              //服务器端口号		
	int localPort = 19739;              //本地端口号
	int result = 0;
	int status = 0;
	char serverIP[20];


	//处理数据线程
	int tempcount = 0;
	HANDLE hThread = NULL;
	HANDLE createTableThread = NULL;
	DATABUFFER*data_buffer;
	pPackageList = QUEUE_Init();
	if (pPackageList == NULL)
	{
		printf("create the package failed!\n");
		exit(1);
	}
	InitializeCriticalSection(&g_cs);


	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		printf("WSAStartup failed!\n");
		exit(1);
	}

	//初始化服务器socket
	serverSockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSockfd == INVALID_SOCKET)
	{
		perror("server");
		exit(1);
	}
	sin = (struct sockaddr_in *) &localAddr;
	memset((char*)sin, '\0', sizeof(localAddr));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(localPort);
	sin->sin_addr.s_addr = INADDR_ANY;
	result = bind(serverSockfd, &localAddr, sizeof(*sin));
	if (result < 0) {
		WriteSystemLog("server sockfd bind error\n");
		exit(1);
	}

	//add address info
	sin = (struct sockaddr_in *) & serverAddr;
	memset((char *)sin, '\0', sizeof(serverAddr));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(serverPort);
	getParamFromConfig("serverIp",serverIP);    //读取server ip
	sin->sin_addr.s_addr = inet_addr(serverIP);

	getParamFromConfig("sendWay", sendWay);      //从配置文件中读取发送SMS方式

	loadMessageFormat(format);                      //读取发送短信的格式
	if (format == NULL) {
		WriteSystemLog("format error!");
		exit(1);
	}

	loginInServer(serverSockfd, serverAddr);        //向服务器注册

	if (!ConnectDatabase()) {                            //连接数据库
		exit(1);
	}
	getMap();                                        //得到映射表

	parseAlarmTable();                               //解析告警数据表
	
	//initRequest();                              //初始化云端发送口
	initSerialPort();                              //初始化DTU串口设置

	//接收告警信息
	for (;;) {

	//	TIMEVAL timeout;
		FD_ZERO(&readfds);
		if (serverSockfd >= 0) {
			FD_SET(serverSockfd, &readfds);
		}
	//	timeout.tv_sec = 10;
	//	timeout.tv_usec = 0;
		status = select(32, &readfds, NULL, NULL,NULL); //block here for 1 sec.
		memset(recv_buffer, '\0', sizeof(recv_buffer));

		if (serverSockfd >= 0 && FD_ISSET(serverSockfd, &readfds))
		{
			int salen = sizeof(serverAddr);
			result = recvfrom(serverSockfd, (char *)recv_buffer,
				(int) sizeof(recv_buffer),
				(int)0, &serverAddr, &salen);
			if (result > 0)
			{
#ifdef DEBUG
				printf("recv_data:%s\n", recv_buffer);
				for (int i = 0; i < result; i++)
					printf("%2x ", recv_buffer[i]);
				printf("\n");
#endif
				data_buffer = transToNode(recv_buffer, result);
				EnterCriticalSection(&g_cs);
				tempcount = pPackageList->m_Count;
				if (QUEUE_AddToTail(pPackageList, &(data_buffer)->next) == FALSE)
				{
			
					LeaveCriticalSection(&g_cs);
					continue;
				}

				else
				{
					if (tempcount == 0)
					{
						if (dataDeal_thread == false)
						{
							if (hThread != NULL)CloseHandle(hThread);
							dataDeal_thread = TRUE;
							hThread = CreateThread(NULL, 0, getAlarmData, NULL, 0, NULL);

						}
					}
					LeaveCriticalSection(&g_cs);
				}
				
			}
		}
	}
	loginOutServer(serverSockfd, serverAddr);  //注销
	closesocket(serverSockfd);  //关闭端口

	AT_ComClose();

	sms_send_message_request_2_free(request);  //关闭请求连接
	sms_send_message_response_2_free(resp);    //关闭响应连接
	FreeConnect();
	WSACleanup();//释放资源的操作
}