#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include "windows.h"
#include "mysql.h"
#include "sms_api.h"
#include "queue.h"
typedef unsigned char   uint8_t;
typedef u_int UINT4;

#define DEBUG
#define MAXSIZE 1024                //��������
#define MAXTELPHONENUM 25              
#define MAXDEVNUM 1024                //����豸��
#define MAXDESTRICTNUM 20            //���������

typedef struct dataBuffer
{
	tQUEUE_NODE next;
	int size;
	char data[1024];
}DATABUFFER;

typedef struct DistrictList {
	int district_id;                        //�����
	char telephones[MAXTELPHONENUM][20];    //�ֻ���
	char dev_gw_cpuid[MAXDEVNUM][30];         //���������������غ�
	char device_name[MAXDEVNUM][50];             //�豸����
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

extern DISTRICTLIST districtList[MAXDESTRICTNUM];
extern int districtListCount;
extern int districtArray[MAXDESTRICTNUM];  //������غ�����Ӧ��Щ����
extern int districtArrayCount;
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

extern MYSQL mysql;


//cloud
extern sms_send_message_request_2_t request;
extern sms_send_message_response_2_t resp;

extern char sendWay[20];
extern char format[512];
extern tQUEUE*pPackageList;
extern CRITICAL_SECTION g_cs;
extern BOOL dataDeal_thread;

void loadAlarmDictionary(char*alarmContent);     //��ȡ�澯�ֵ�
void loadUserList(char*userList);                 //��ȡע���û���
void loadMessageFormat(char*content);
void getCurFilePath(char*fileName, char*strFileName);
void getParamFromConfig(const char*optionName,char*value);     //�������÷�����Ӧֵ
void WriteSystemLog(const char * strContent);      //��־
void parseStrToChars(const char*data,int*chars);          //���ַ�Э���е�����תΪunsigned char��
void loginInServer(int sockfd,struct sockaddr serverAddr);  //��ע����Ϣ��������
void loginOutServer(int sockfd, struct sockaddr serverAddr); //�˳�������
DATABUFFER* transToNode(char*data,int length);    //�����յ�������ת��Ϊ���нڵ�
char *replaceStr(char *str, char *oldstr, char *newstr);
int CheckByteCmd(char* command, int len);
DataPacket *  parse(char receive[]);
#endif