
//#include "stdafx.h"
#include "list.h"
/************************************************************
***creat the queue and initilize the queue.
**************************************************************/
tLIST* LIST_Init( )
{ 
	tLIST* pList = (tLIST*)malloc(sizeof(tLIST));
	if (!pList) 	return NULL;
	else{
		(pList)->Head.pNext = &(pList)->Head; 
		(pList)->Tail       = &(pList)->Head; 
		(pList)->m_Count   = 0;
		return pList;
	}
}
/************************************************************
***destroy  the LIST
**************************************************************/
BOOL LIST_Destroy(tLIST *pList)
{ 
	while((pList)->Head.pNext){

		(pList)->Tail = ((pList)->Head.pNext)->pNext;
		free((pList)->Head.pNext);
		(pList)->Head.pNext = (pList)->Tail;
	}
	free(pList);
	return TRUE;
}

/************************************************************
***add the new node to  the list p place .//insert after the number p node 
**************************************************************/
//BOOL QUEUE_AddToTail(tQUEUE*pList, tQUEUE_NODE*pNode)
BOOL LIST_Insert(tLIST*pList, u_int p,tLIST_NODE*pNode)
{  
	/*
	if((pList)->Head.pNext == &(pList)->Head) {
	(pList)->Head.pNext = pNode;
	(pList)->Tail = pNode;
	pList->m_Count++;
	}else{
	(pList->Tail)->pNext = pNode;
	(pList)->Tail = pNode;
	pList->m_Count++;
	}
	*/
	int i = 0;
	tLIST_NODE *ptmpNode;
	if(p> pList->m_Count)return FALSE; //no success.
	if(p==pList->m_Count)
	{
		if(p==0)//insert the head .
		{
			(pList)->Head.pNext = pNode;
			(pList)->Tail = pNode;
			pList->m_Count++;
		}else{ //insert the tail.
			(pList->Tail)->pNext = pNode;
			(pList)->Tail = pNode;
			pList->m_Count++;
		}
	}
	else //insert the middle.
	{
		// if p <0 return FALSE;
		///find the insert point .
		ptmpNode = (pList)->Head.pNext;
		for(unsigned i = 0;i<p;i++)
		{
			if( i < p - 1 )
				ptmpNode  = ptmpNode->pNext;
		}
		pNode->pNext = ptmpNode->pNext;
		ptmpNode->pNext = pNode;
		pList->m_Count++;
	}
	return TRUE;
}
/************************************************************
***delete the  node from  the list  after the p number .
p  = 0,=number ,<number,>number.
**************************************************************/
BOOL LIST_DelNode(tLIST*pList,u_int p)
{
	int i = 0;
	tLIST_NODE *ptmpNode;
	tLIST_NODE * ptemp;
	//	if((pList)->Head.pNext != &(pList)->Head) 
	if(p> pList->m_Count||p == 0)return FALSE; //no success.
	if(p == pList->m_Count)
	{

		if(pList->Head.pNext == pList->Tail) //number is 1
		{
			(pList)->Head.pNext = &(pList)->Head; 
			(pList)->Tail       = &(pList)->Head; 
			(pList)->m_Count   = 0;
			return TRUE;
		}
		else//>1 del the tail.
		{  
			(pList)->Head.pNext = (pList->Head.pNext)->pNext;
			pList->m_Count -- ;
		}
	}else //1=< p <  number
	{
		ptmpNode = (pList)->Head.pNext;
		for( unsigned i=0;i<p;i++)
		{
			if(i<p-1) ptmpNode  = ptmpNode->pNext;
		}
		ptemp = ptmpNode->pNext;

		ptmpNode->pNext = ptmpNode->pNext->pNext;
		pList->m_Count -- ;
		free(ptemp);
		return TRUE;
	}

	return FALSE;
}

/************************************************************
***get the  first node from  the queue  head
output to the pNode point.
**************************************************************/
