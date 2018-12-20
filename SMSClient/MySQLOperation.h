#ifndef MYSQLOPERATION_H
#define MYSQLOPERATION_H

#include <stdbool.h>
#include "mysql.h"
#include "global.h"

extern MYSQL mysql;
DISTRICTLIST districtList[MAXDESTRICTNUM];
int districtListCount;

void initdistrictList();
bool ConnectDatabase();  //连接数据库
void FreeConnect();   //断开连接
MYSQL_ROW QueryProcess(char* sql_sentence);
void getDevCpuID(int destrict_id, DISTRICTLIST *districtList, int index);
void getMap();

#endif // !1
