//#include "stdafx.h"
#include "global.h"
#include "TranslateToSMS.h"
#include "SendSMS.h"
#include "MySQLOperation.h"
//#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

extern DCB m_dcb;
extern HANDLE m_hComm;
extern char sendWay[20];
int main()
{
	WSADATA WSAData;
	struct sockaddr serverAddr;
	struct sockaddr_in *sin;
	fd_set			readfds;
	int serverSockfd;
	char recv_buffer[1024];
	char alarmContent[MAXSIZE];        //告警内容
	char alarmType[25];
	int serverPort= 13245;              //服务器端口号		
	int result = 0;
	int status = 0;
	char serverIP[20];

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
	//add address info
	sin = (struct sockaddr_in *) & serverAddr;
	memset((char *)sin, '\0', sizeof(serverAddr));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(serverPort);
	getParamFromConfig("serverIp",serverIP);    //读取server ip
	sin->sin_addr.s_addr = inet_addr(serverIP);
	/*result = bind(serverSockfd, &serverAddr, sizeof(*sin));
	if (result < 0) {
		perror("bind");
		exit(1);
	}*/

	getParamFromConfig("sendWay", sendWay);      //从配置文件中读取发送SMS方式

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
				/*for (int i = 0; i < result; i++) {
					printf("%02x", recv_buffer[i]);
				}*/
#endif

				memset(alarmContent, '\0', MAXSIZE);
				memset(alarmType, '\0', 25);
				getAlarmMessage(recv_buffer,result,alarmContent,alarmType);
#ifdef DEBUG
				printf("%s", alarmContent);
#endif

				
			}
		}
	}
	loginOutServer(serverSockfd, serverAddr);  //注销
	closesocket(serverSockfd);  //关闭端口

	CloseHandle(m_hComm);   //关闭串口句柄

	sms_send_message_request_2_free(request);  //关闭请求连接
	sms_send_message_response_2_free(resp);    //关闭响应连接
	FreeConnect();
	WSACleanup();//释放资源的操作
}