//#include "stdafx.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void getCurFilePath(char*fileName, char*strFileName)
{
	char szFileName[_MAX_PATH];
	//	char strFileName[_MAX_PATH];
	GetModuleFileNameA(NULL, szFileName, sizeof(szFileName));

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(szFileName, drive, dir, fname, ext);

	sprintf(strFileName, "%s%s%s", drive, dir, fileName);
}

void getParamFromConfig(const char * optionName, char*value)
{
	char fileName[_MAX_PATH];
	memset(fileName, '\0', _MAX_PATH);
	getCurFilePath("SMSClient.cfg", fileName);
	FILE *fp = NULL;
	fp = fopen(fileName, "r");
	if (!fp) {
		printf("File could not be opened! \n");
		exit(1);
	}
	char line[100];

	while (fgets(line, sizeof(line), fp)) {

		char name[50];
		memset(value, '\0', 20);
		if (sscanf(line, "%s%s", name, value) != 2) {
			WriteSystemLog("config error");
			exit(1);
		}
		else if (strcmp(optionName, name) == 0) {
			fclose(fp);
			fp = NULL;
			return;
		}

	}
}

void loadAlarmDictionary(char*alarmContent)
{
	char fileName[260];
	memset(fileName, '\0', 260);
	getCurFilePath("alarmDictionary.txt", fileName);

	FILE *fp;
	fp = fopen(fileName, "r");
	if (!fp) {
		WriteSystemLog("alarmDictionary could not be opened! \n");
		exit(1);
	}
	int i = 0;
	char cChar = fgetc(fp);
	while (!feof(fp))
	{
		*(alarmContent + i) = cChar; i++; cChar = fgetc(fp);
	}
	*(alarmContent + i) = '\0';
}

void loadUserList(char * userList)
{
	char fileName[260];
	memset(fileName, '\0', 260);
	getCurFilePath("userList.txt", fileName);

	FILE *fp;
	fp = fopen(fileName, "r");
	if (!fp) {
		WriteSystemLog("userList could not be opened! \n");
		exit(1);
	}
	int i = 0;
	char cChar = fgetc(fp);
	while (!feof(fp))
	{
		*(userList + i) = cChar; i++; cChar = fgetc(fp);
	}
	*(userList + i) = '\0';
}

void WriteSystemLog(const char * strContent)
{
	char szFileName[_MAX_PATH];
	char strFileName[_MAX_PATH];
	GetModuleFileNameA(NULL, szFileName, sizeof(szFileName));

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath(szFileName, drive, dir, fname, ext);
	time_t now;
	struct tm *ptm_now;
	now = time(NULL);
	ptm_now = localtime(&now);
	sprintf(strFileName, "%s%sSMS%02d%02d.log", drive, dir, ptm_now->tm_mon + 1, ptm_now->tm_mday);
	FILE *fp;
	fp = fopen(strFileName, "a");
	if (!fp)
	{
		printf("File could not be opened ");
	}
	fprintf(fp, "%d-%d-%d ", ptm_now->tm_year + 1900, ptm_now->tm_mon + 1, ptm_now->tm_mday);
	fprintf(fp, "%d:%d:%d\t", ptm_now->tm_hour, ptm_now->tm_min, ptm_now->tm_sec);
	fprintf(fp, "%s\n", strContent);

	fclose(fp);
}

void parseStrToInt(char * data, int*dataInt)
{
	char*p;
	int i = 1;
	dataInt[0] = atoi(strtok(data, ","));
	while ((p = strtok(NULL, ",")))
	{
		dataInt[i++] = atoi(p);
	}
}

void loginInServer(int sockfd, struct sockaddr serverAddr)
{
	//	timing_ = time(NULL);
	char message[10];
	memset(message, '\0', sizeof(message));
	memcpy(message, "login in", 10);
	sendto(sockfd, (char*)message, (int)sizeof(message), (int)0, &serverAddr, sizeof(struct sockaddr_in));
}

void loginOutServer(int sockfd, struct sockaddr serverAddr)
{
	char message[20];
	memset(message, '\0', sizeof(message));
	memcpy(message, "login out", 20);
	sendto(sockfd, (char*)message, (int)sizeof(message), (int)0, &serverAddr, sizeof(struct sockaddr_in));
}

//void keepAliveToServer(int sockfd, struct sockaddr serverAddr)
//{
//	double duration = difftime(time(NULL), (time_t)timing_);
//}

DataPacket *  parse(char receive[])
{
	//struct Datapack record={{'\0'},{'\0'},{'\0'},{'\0'},{'\0'},{'\0'},{'\0'},{'\0'}};
	DataPacket * temp;


	if ((temp = (DataPacket *)malloc(sizeof(DataPacket))) ==
		(DataPacket *)NULL) {
		//log(L_ERR|L_CONS, "no memory");
		printf("no memory");
		exit(1);
	}
	memset(temp, 0, sizeof(DataPacket));

	//struct Datapack record;
	///memset(&record ,-1,sizeof(record));
	char a[3][2048] = { '\0' };//   2048 is > (3+1)*40 *10
	char b[4][100] = { '\0' };
	char *p;
	int i = 0;
	//PRT_INFO("%s \n",receive);
	strcpy(temp->FH, strtok(receive, " "));
	while ((p = strtok(NULL, " ")))
	{
		strcpy(a[i++], p);
	}
	strcpy(temp->FE, a[2]);
	i = 0;
	strcpy(temp->NSID, strtok(a[0], ","));
	while ((p = strtok(NULL, ",")))
	{
		strcpy(b[i++], p);
	}
	strcpy(temp->ADSTB, b[0]);
	strcpy(temp->AdT, b[1]);
	strcpy(temp->morder, b[2]);
	strcpy(temp->sorder, b[3]);

	strcpy(temp->data, a[1]);


	return temp;
}
//判断命令是字节命令还是字符命令
//return: true - 字节命令；false - 字符命令
int CheckByteCmd(char* command, int len)
{
	char cmdTmp[1024];
	memset(cmdTmp, 0, 1024);
	memcpy(cmdTmp, command, len > 1024 ? 1024 : len);

	if (32 != cmdTmp[10] && 's' != cmdTmp[10] && 'q' != cmdTmp[10] && ':' != cmdTmp[10])			//第11个字节不是字符空格、s、q、:，命令一定为字节命令
	{
		return 1;
	}

	const char* split = " ";
	char* p = strtok(cmdTmp, split);
	int count = 0;
	char str1[1024], str4[1024];
	memset(str1, 0, 1024);
	memset(str4, 0, 1024);
	return 0;
}