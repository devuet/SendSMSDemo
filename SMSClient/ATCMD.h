#ifndef _ATCMD_H_

#define _ATCMD_H_



#ifdef __cplusplus

extern "C"

{

#endif



#include <windows.h>
#include <stdio.h>  
#include   <stdlib.h>
#include <string.h>  
#include <conio.h>  
#include <winnt.h>  


	//#pragma   comment(lib, "libcomm.lib")

	/* 串口动态链接库接口*/

	_declspec(dllimport) HANDLE SerialOpen(char *name);

	_declspec(dllimport) int SerialSet(HANDLE fd, int rate);

	_declspec(dllimport) int SerialSend(HANDLE fd, char *data, int len);

	//_declspec(dllimport) int SerialSendWithTimeout(HANDLE fd, char *data, int timeout);

	_declspec(dllimport) int SerialRecv(HANDLE fd, char *data);

	//	_declspec(dllimport) int SerialRecvWithTimeout(HANDLE fd, char *data, int timeout);

	_declspec(dllimport) int SerialClose(HANDLE fd);


	/* AT 指令对外接口*/
	_declspec(dllexport) int AT_ComOpen();

	_declspec(dllexport) int AT_ComClose();

	_declspec(dllexport) int AT_ComConfig(char *name, int rate);

	_declspec(dllexport) int AT_ApnConfig(char *apn);

	_declspec(dllexport) int AT_SendSMS(char *phonenum,char *msg, int len);

	_declspec(dllexport) int AT_GetFlux(char *rdata, int buf_len);

	_declspec(dllexport) int AT_GetCi(char *rdata, int buf_len);

	_declspec(dllexport) int at_test(char *wdata, char *rdata, int len);



#ifdef __cplusplus

}

#endif



#endif
