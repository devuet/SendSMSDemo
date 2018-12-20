#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include "windows.h"
#include "mysql.h"
#include "sms_api.h"

typedef unsigned char   uint8_t;
typedef u_int UINT4;

#define DEBUG
#define MAXSIZE 1024
#define MAXTELPHONENUM 25
#define MAXDEVNUM 1024
#define MAXDESTRICTNUM 20


typedef struct DistrictList {
	int district_id;                        //�����
	char telephones[MAXTELPHONENUM][20];    //�ֻ���
	char dev_cpuid[MAXDEVNUM][20];         //���������������غ�
	int telCount;
	int devCount;
}DISTRICTLIST;


typedef struct Datapack
{
	char  FH[16];    //֡ͷ
	char  NSID[10];   //�����
	char  ADSTB[10]; //��ַ��
	char  AdT[5];//��ַ����
	char  morder[10];//������
	char  sorder[10];//������
	char  data[1024];
	char  FE[10];//֡β
}DataPacket;

DISTRICTLIST districtList[MAXDESTRICTNUM];
int districtListCount;

//�ֽ�Э��֡��ʽ�ṹ(Device To Server)
#pragma pack(1)	
typedef struct PackDev
{
	int devCpuId;		//֡ͷ,�豸CPU_ID
	char netId;			//����ź͵�ַ���ͣ���6 bitΪ����ţ���2 bitΪ��ַ����
	short addr;			//��ַ��2 Byte
	unsigned short len;	//���ݳ���, 2 Byte
	char data[1024];			//���ݽṹָ��
}PACKDEV;
#pragma pack()

MYSQL mysql;

//DTU
DCB m_dcb;
HANDLE m_hComm;   //CreateFile�����Ĵ��ھ��

//cloud
sms_send_message_request_2_t request;
sms_send_message_response_2_t resp;

char sendWay[20];

void loadAlarmDictionary(char*alarmContent);     //��ȡ�澯�ֵ�
void loadUserList(char*userList);                 //��ȡע���û���
void getCurFilePath(char*fileName, char*strFileName);
void getParamFromConfig(const char*optionName,char*value);     //�������÷�����Ӧֵ
void WriteSystemLog(const char * strContent);      //��־
void parseStrToInt(char*data,int*dataInt);          //���ַ�Э���е�����תΪint��
void loginInServer(int sockfd,struct sockaddr serverAddr);  //��ע����Ϣ��������
void loginOutServer(int sockfd, struct sockaddr serverAddr); //�˳�������


#endif