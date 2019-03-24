#include "MySQLOperation.h"
#include "cJSON.h"
#include "global.h"

//初始化映射表
void initdistrictList()
{
	int i, j;
	districtListCount = 0;
	for (i = 0; i < MAXDESTRICTNUM; i++) {
		districtList[i].devCount = 0;
		districtList[i].telCount = 0;
		for (j = 0; j < MAXDEVNUM; j++) {
			memset(districtList[i].dev_gw_cpuid[j], '\0', 30);
			memset(districtList[i].device_name[j], '\0', 50);
		}
		for (j= 0; j < MAXTELPHONENUM; j++) {
			memset(districtList[i].telephones[j], '\0', 20);
		}
	}
}

bool ConnectDatabase()
{
	mysql_library_init(NULL, 0, 0);
	mysql_init(&mysql);
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");
	char server[20], user[20], password[20], database[20];
	getParamFromConfig("server", server);
	getParamFromConfig("user", user);
	getParamFromConfig("password", password);
	getParamFromConfig("database", database);
	if (!mysql_real_connect(&mysql, server, user, password, database, 33306, NULL,NULL)) {
		mysql_close(&mysql);
		WriteSystemLog(mysql_error(&mysql));
		return false;
	}
	return true;
}

void FreeConnect()
{
	if (NULL == &mysql)
	{
		return;
	}
	mysql_close(&mysql);
}

MYSQL_ROW QueryProcess(char * sql_sentence)
{
	if (mysql_query(&mysql, sql_sentence))        //执行SQL语句
	{

		WriteSystemLog(mysql_error(&mysql));
		return NULL;
	}
	else
	{
		MYSQL_RES* res = mysql_store_result(&mysql);
		if (NULL != res)
		{
			return mysql_fetch_row(res);
		}
		return NULL;
	}
}

void getDevCpuID(int destrict_id, DISTRICTLIST *districtList, int index)
{
	char query[1024];
	int iIndex = 0;
	char dev_cpuid[30];
	sprintf(query,"select DISTINCT dev_gw_cpuid from t_device where district_id LIKE '%%%d%%'", destrict_id);
	if (mysql_query(&mysql, query))        //执行SQL语句
	{
		WriteSystemLog(mysql_error(&mysql));
		return;
	}
	else
	{
		MYSQL_RES* res = mysql_store_result(&mysql);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(res)) {
				memset(dev_cpuid, '\0', 30);
				sprintf(dev_cpuid, "%s", row[0]);
				if (strlen(dev_cpuid) == 10) {     //筛选掉1361147666+0+2类型，无意义的dev_gw_cpuid
					iIndex = districtList[index].devCount;
					sprintf(districtList[index].dev_gw_cpuid[iIndex], "%s", row[0]);
					sprintf(query, "select device_name from t_device where dev_gw_cpuid= '%s'", districtList[index].dev_gw_cpuid[iIndex]);
					MYSQL_ROW row = QueryProcess(query);
					if (row != NULL) {
						char nameutf8[50] = { 0 };
						sprintf(nameutf8, "%s", row[0]);
						sprintf(districtList[index].device_name[iIndex], "%s", nameutf8);
					}
					districtList[index].devCount++;
				}
//				else {
//#ifdef DEBUG
//					printf("useless dev_gw_cpuid: %s\n", dev_cpuid);
//#endif
//				}
		}
	}
}

int getDistrictIndexAndName(const char * cpuID,char*devName)
{
	districtArrayCount = 0;
	memset(districtArray, '\0', MAXDESTRICTNUM);
	int i, j;
	for (i = 0; i < districtListCount; i++) {
		for (j = 0; j < districtList[i].devCount; j++) {
			if (strcmp(cpuID, districtList[i].dev_gw_cpuid[j]) == 0) {
				districtArray[districtArrayCount++] = i;
				sprintf(devName, "%s", districtList[i].device_name[j]);
				break;
			}
		}
	}
	return districtArrayCount;
}

void getMap()
{
	char userList[2048];
	char userName[20];
	char query[1024];
	int district_id = 0;
	int index = 0;
	int i, j;
	char telephone[20];
	MYSQL_ROW row;
	bool existFlag;  //标志destrict是否在列表中
	memset(userList, '\0', 2048);
	loadUserList(userList);
	cJSON*root = cJSON_Parse(userList);
	cJSON*item = cJSON_GetObjectItem(root, "user");
	int size = cJSON_GetArraySize(item);

	initdistrictList();                               //初始化districtList表

	for (i = 0; i < size; i++) {                      //建立映射关系
		memset(userName, '\0', 20);
		memset(query, '\0', 1024);
		memset(telephone, '\0', 20);
		sprintf(userName, "%s", cJSON_GetArrayItem(item, i)->valuestring);
		sprintf(query, "select district_id,telephone from v_user where user_id= '%s' ", userName);    //根据userList表中的用户名得到电话号码和区域号
		row = QueryProcess(query);
		if (row != NULL) {                       //单位和电话号码不能为空

			existFlag = false;
			district_id = atoi(row[0]);
			sprintf(telephone, "%s", row[1]);

			//检查此区域id是否已在districtList中
			for (j = 0; j < districtListCount; j++) {
				if (districtList[j].district_id == district_id) {
					existFlag = true;
					index = districtList[j].telCount;
					sprintf(districtList[j].telephones[index], "%s", telephone);
					districtList[j].telCount++;
					break;
				}
			}
			
			if (existFlag==false) {
				districtList[districtListCount].district_id = district_id;
				index = districtList[districtListCount].telCount;
				sprintf(districtList[districtListCount].telephones[index], "%s", telephone);
				districtList[districtListCount].telCount++;
				getDevCpuID(district_id, districtList, i);
				districtListCount++;
			}
		}

	}
	cJSON_Delete(root); //从内存中删除userList

}
