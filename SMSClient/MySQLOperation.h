#ifndef MYSQLOPERATION_H
#define MYSQLOPERATION_H

#include <stdbool.h>
#include "mysql.h"
#include "global.h"

MYSQL mysql;
DISTRICTLIST districtList[MAXDESTRICTNUM];
int districtListCount;

int districtArray[MAXDESTRICTNUM];  //存放网关号所对应哪些区域
int districtArrayCount;

void initdistrictList();
bool ConnectDatabase();  //连接数据库
void FreeConnect();   //断开连接
MYSQL_ROW QueryProcess(char* sql_sentence);
void getDevCpuID(int destrict_id, DISTRICTLIST *districtList, int index);
int getDistrictIndexAndName(const char*cpuID,char*devName);  
void getMap();

#endif // !1
