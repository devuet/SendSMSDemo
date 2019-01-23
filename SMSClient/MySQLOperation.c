#include "MySQLOperation.h"
#include "cJSON.h"
#include "global.h"

//��ʼ��ӳ���
void initdistrictList()
{
	int i, j;
	districtListCount = 0;
	for (i = 0; i < MAXDESTRICTNUM; i++) {
		districtList[i].devCount = 0;
		districtList[i].telCount = 0;
		for (j = 0; j < MAXDEVNUM; j++) {
			memset(districtList[i].dev_cpuid[j], '\0', 20);
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
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "utf8");
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
	if (mysql_query(&mysql, sql_sentence))        //ִ��SQL���
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
	sprintf(query,"select DISTINCT dev_cpuid from t_device where district_id LIKE '%%%d%%'", destrict_id);
	if (mysql_query(&mysql, query))        //ִ��SQL���
	{
		WriteSystemLog(mysql_error(&mysql));
		return NULL;
	}
	else
	{
		MYSQL_RES* res = mysql_store_result(&mysql);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(res)) {

				iIndex = districtList[index].devCount;
				sprintf(districtList[index].dev_cpuid[iIndex], "%s", row[0]);
				districtList[index].devCount++;
		}
	}
}

int getDistrictIndex(const char * cpuID)
{
	districtArrayCount = 0;
	memset(districtArray, '\0', MAXDESTRICTNUM);
	int i, j;
	for (i = 0; i < districtListCount; i++) {
		for (j = 0; j < districtList[i].devCount; j++) {
			if (strcmp(cpuID, districtList[i].dev_cpuid[j]) == 0) {
				districtArray[districtArrayCount++] = i;
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
	bool existFlag;  //��־destrict�Ƿ����б���
	memset(userList, '\0', 2048);
	loadUserList(userList);
	cJSON*root = cJSON_Parse(userList);
	cJSON*item = cJSON_GetObjectItem(root, "user");
	int size = cJSON_GetArraySize(item);

	initdistrictList();                               //��ʼ��districtList��

	for (i = 0; i < size; i++) {                      //����ӳ���ϵ
		memset(userName, '\0', 20);
		memset(query, '\0', 1024);
		memset(telephone, '\0', 20);
		sprintf(userName, "%s", cJSON_GetArrayItem(item, i)->valuestring);
		sprintf(query, "select district_id,telephone from v_user where user_id= '%s' ", userName);    //����userList���е��û����õ��绰����������
		row = QueryProcess(query);
		if (row != NULL) {                       //��λ�͵绰���벻��Ϊ��

			existFlag = false;
			district_id = atoi(row[0]);
			sprintf(telephone, "%s", row[1]);

			//��������id�Ƿ�����districtList��
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

}
