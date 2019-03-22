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
	int serverPort= 19732;              //�������˿ں�		
	int localPort = 19739;              //���ض˿ں�
	int result = 0;
	int status = 0;
	char serverIP[20];


	//���������߳�
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

	//��ʼ��������socket
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
	getParamFromConfig("serverIp",serverIP);    //��ȡserver ip
	sin->sin_addr.s_addr = inet_addr(serverIP);

	getParamFromConfig("sendWay", sendWay);      //�������ļ��ж�ȡ����SMS��ʽ

	loadMessageFormat(format);                      //��ȡ���Ͷ��ŵĸ�ʽ
	if (format == NULL) {
		WriteSystemLog("format error!");
		exit(1);
	}

	loginInServer(serverSockfd, serverAddr);        //�������ע��

	if (!ConnectDatabase()) {                            //�������ݿ�
		exit(1);
	}
	getMap();                                        //�õ�ӳ���

	parseAlarmTable();                               //�����澯���ݱ�
	
	//initRequest();                              //��ʼ���ƶ˷��Ϳ�
	initSerialPort();                              //��ʼ��DTU��������

	//���ո澯��Ϣ
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
	loginOutServer(serverSockfd, serverAddr);  //ע��
	closesocket(serverSockfd);  //�رն˿�

	AT_ComClose();

	sms_send_message_request_2_free(request);  //�ر���������
	sms_send_message_response_2_free(resp);    //�ر���Ӧ����
	FreeConnect();
	WSACleanup();//�ͷ���Դ�Ĳ���
}