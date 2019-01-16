/*
//define the list used in the master thread.
//the queue will be used as a buffer for the package get from the socket port.
//
//time: 20160310
//editor :liuxing 
*/
#include "stdafx.h"
#include "windows.h"
#include "com.h"
#ifndef _LIST_H
#define _LIST_H


/************************************ 
*** Type Definition for LIST_NODE ****
*************************************/
typedef struct LIST_NODE{
        struct LIST_NODE *pNext; /**** Points At The Next Node In The List ****/
        }tLIST_NODE;

/*******************************************************
*** Type Definition For SLL Structure               ****
*** List is Organised as Singly Linked Cicular List ****
********************************************************/
typedef struct XLIST{
		tLIST_NODE  Head;     /**** Header List Node *****/
        tLIST_NODE *Tail;    /**** Tail Node        *****/
        UINT4       m_Count; /**** Number Of Nodes In List ****/
        }tLIST;

/************************************************************
***creat the LIST and initilize the LIST.
**************************************************************/
  tLIST* LIST_Init();
/************************************************************
***destroy  the LIST 
**************************************************************/
 BOOL LIST_Destroy(tLIST *pList);
/************************************************************
***add the new node to  the LIST  tail
**************************************************************/
BOOL LIST_Insert(tLIST*pList, u_int p,tLIST_NODE*pNode);
/************************************************************
***delete the  node from  the LIST
**************************************************************/
BOOL LIST_DelNode(tLIST*pList,u_int p);

#endif