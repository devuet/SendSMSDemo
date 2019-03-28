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
#define MAXSIZE 1024                //短信内容
#define MAXTELPHONENUM 25              
#define MAXDEVNUM 1024                //最大设备数
#define MAXDESTRICTNUM 20            //最大区域数

typedef struct dataBuffer
{
	tQUEUE_NODE next;
	int size;
	char data[1024];
}DATABUFFER;

typedef struct DistrictList {
	int district_id;                        //区域号
	char telephones[MAXTELPHONENUM][20];    //手机号
	char dev_gw_cpuid[MAXDEVNUM][30];         //区域所包含的网关号
	char device_name[MAXDEVNUM][50];             //设备名称
	int telCount;
	int devCount;
}DISTRICTLIST;


typedef struct Datapack
{
	char  FH[16];    //帧头
	char  NSID[10];   //网络号
	char  ADSTB[10]; //地址号
	char  AdT[5];//地址类型
	char  morder[10];//主命令
	char  sorder[10];//子命令
	char  data[1024];
	char  FE[10];//帧尾
}DataPacket;

extern DISTRICTLIST districtList[MAXDESTRICTNUM];
extern int districtListCount;
extern int districtArray[MAXDESTRICTNUM];  //存放网关号所对应哪些区域
extern int districtArrayCount;
//字节协议帧格式结构(Device To Server)
#pragma pack(1)	
typedef struct PackDev
{
	int devCpuId;		//帧头,设备CPU_ID
	char netId;			//网络号和地址类型，高6 bit为网络号，低2 bit为地址类型
	short addr;			//地址，2 Byte
	unsigned short len;	//数据长度, 2 Byte
	char data[1024];			//数据结构指针
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

void loadAlarmDictionary(char*alarmContent);     //读取告警字典
void loadUserList(char*userList);                 //读取注册用户表
void loadMessageFormat(char*content);
void getCurFilePath(char*fileName, char*strFileName);
void getParamFromConfig(const char*optionName,char*value);     //根据配置返回相应值
void WriteSystemLog(const char * strContent);      //日志
void parseStrToChars(const char*data,int*chars);          //将字符协议中的数据转为unsigned char型
void loginInServer(int sockfd,struct sockaddr serverAddr);  //发注册消息给服务器
void loginOutServer(int sockfd, struct sockaddr serverAddr); //退出服务器
DATABUFFER* transToNode(char*data,int length);    //将接收到的数据转化为队列节点
char *replaceStr(char *str, char *oldstr, char *newstr);
int CheckByteCmd(char* command, int len);
DataPacket *  parse(char receive[]);
#endif