#ifndef MYSQLOPERATION_H
#define MYSQLOPERATION_H

#include <stdbool.h>
#include "mysql.h"
#include "global.h"

MYSQL mysql;
DISTRICTLIST districtList[MAXDESTRICTNUM];
int districtListCount;

int districtArray[MAXDESTRICTNUM];  //������غ�����Ӧ��Щ����
int districtArrayCount;

void initdistrictList();
bool ConnectDatabase();  //�������ݿ�
void FreeConnect();   //�Ͽ�����
MYSQL_ROW QueryProcess(char* sql_sentence);
void getDevCpuID(int destrict_id, DISTRICTLIST *districtList, int index);
int getDistrictIndexAndName(const char*cpuID,char*devName);  
void getMap();

#endif // !1
