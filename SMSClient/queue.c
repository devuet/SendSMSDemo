
//#include "stdafx.h"
#include "queue.h"
/************************************************************
***creat the queue and initilize the queue.
**************************************************************/
tQUEUE* QUEUE_Init( )
{ 
    tQUEUE* pList = (tQUEUE*)malloc(sizeof(tQUEUE));
    if (!pList) 	return NULL;
	else{
	(pList)->Head.pNext = &(pList)->Head; 
        (pList)->Tail       = &(pList)->Head; 
        (pList)->m_Count   = 0;
	return pList;
	}
}
/************************************************************
***destroy  the queue 
**************************************************************/
BOOL QUEUE_Destroy(tQUEUE *pList)
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
***add the new node to  the queue  tail
**************************************************************/
BOOL QUEUE_AddToTail(tQUEUE*pList, tQUEUE_NODE*pNode)
{  
	if((pList)->Head.pNext == &(pList)->Head)
	{
		(pList)->Head.pNext = pNode;
		(pList)->Tail = pNode;
		pList->m_Count++;
	}
	else
	{
		(pList->Tail)->pNext = pNode;
		(pList)->Tail = pNode;
		pList->m_Count++;
	}
	return TRUE;
}
/************************************************************
***delete the  node from  the queue  head
**************************************************************/
BOOL QUEUE_DelHead(tQUEUE*pList)
{
	if((pList)->Head.pNext != &(pList)->Head) {
	
	  if(pList->Head.pNext == pList->Tail){
	  (pList)->Head.pNext = &(pList)->Head; 
        (pList)->Tail       = &(pList)->Head; 
        (pList)->m_Count   = 0;
		return TRUE;
	  }
	  else{
	    (pList)->Head.pNext = (pList->Head.pNext)->pNext;
		pList->m_Count -- ;
        return TRUE;
	  }//end if only one node exist.
	}// end if  no one node exist. 
	return FALSE;
}

/************************************************************
***get the  first node from  the queue  head
output to the pNode point.
**************************************************************/
tQUEUE_NODE* QUEUE_GetFirst(tQUEUE*pList)
{
   tQUEUE_NODE	*pNode;
   if((pList)->m_Count != 0 ) {
     pNode = (pList)->Head.pNext;
   return pNode;
   } //end  if no one node exist.
   else
	   return NULL;
}