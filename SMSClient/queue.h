/*
//difine the queue used in the master thread.
//the queue will be used as a buffer for the package get from the socket port.
//
//time: 20020321
//editor :liuxing 
*/
#include "stdafx.h"
#include "windows.h"
#include "com.h"
#ifndef _QUEUE_H
#define _QUEUE_H


/************************************ 
*** Type Definition for QUEUE_NODE ****
*************************************/
typedef struct QUEUE_NODE{
        struct QUEUE_NODE *pNext; /**** Points At The Next Node In The List ****/
        }tQUEUE_NODE;

/*******************************************************
*** Type Definition For SLL Structure               ****
*** List is Organised as Singly Linked Cicular List ****
********************************************************/
typedef struct QUEUE{
		tQUEUE_NODE  Head;     /**** Header List Node *****/
        tQUEUE_NODE *Tail;    /**** Tail Node        *****/
        UINT4       m_Count; /**** Number Of Nodes In List ****/
        }tQUEUE;

/************************************************************
***creat the queue and initilize the queue.
**************************************************************/
  tQUEUE* QUEUE_Init( );
/************************************************************
***destroy  the queue 
**************************************************************/
 BOOL QUEUE_Destroy(tQUEUE *pList);
/************************************************************
***add the new node to  the queue  tail
**************************************************************/
BOOL QUEUE_AddToTail(tQUEUE*pList, tQUEUE_NODE*pNode);
/************************************************************
***delete the  node from  the queue  head
**************************************************************/
BOOL QUEUE_DelHead(tQUEUE*pList);
/************************************************************
***get the  first node from  the queue  head
output to the pNode point.
**************************************************************/
tQUEUE_NODE *QUEUE_GetFirst(tQUEUE*pList);

#endif