#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"

FIFO * fifo_new_list()
{
	FIFO * li = (FIFO*)malloc(sizeof(FIFO));
	if(li!=NULL)
	{
		li->pHeadPos = NullPos;
		li->pTailPos = NullPos;
		li->pCurrentPos = NullPos;
	}
	return li;
}
void fifo_delete_list(FIFO * li)
{
	if(li==NULL)
		return;
	ElementNode node = fifo_pop_node(li);
	while(node)
	{
		memory_delete_data(node);
		node = fifo_pop_node(li);
	}
	free(li);
}
void fifo_release_data(FIFO * li)
{
	if(li==NULL)
		return;
	ElementNode node = fifo_pop_node(li);
	while(node)
	{
		memory_delete_data(node);
		node = fifo_pop_node(li);
	}
}

ElementNode fifo_get_head(FIFO * li)
{
	if(li==NULL)
		return NullNode;
	li->pCurrentPos = li->pHeadPos;
	if(li->pCurrentPos==NullPos)
		return NullNode;
	return li->pCurrentPos->CurrentNode;
}
ElementNode fifo_get_next(FIFO * li)
{
	if(li==NULL)
		return NullNode;
	if(li->pCurrentPos==NullPos)
		return NullNode;
	li->pCurrentPos = (Position*)li->pCurrentPos->pNextPos;
	if(li->pCurrentPos==NullPos)
		return NullNode;
	return li->pCurrentPos->CurrentNode;
}
ElementNode fifo_get_last(FIFO * li)
{
	if(li==NULL)
		return NullNode;
	li->pCurrentPos = li->pTailPos;
	if(li->pCurrentPos==NullPos)
		return NullNode;
	return li->pCurrentPos->CurrentNode;
}
void fifo_push_node(ElementNode node,FIFO * li)
{
	if(li==NULL)
		return;
	Position * pos = (Position*)malloc(sizeof(Position));
	if(pos==NULL)
		return;

	pos->CurrentNode = node;
	pos->pPrevPos = li->pTailPos;
	pos->pNextPos = NullPos;
	if(li->pTailPos==NullPos)
		li->pTailPos = pos;
	else
	{
		li->pTailPos->pNextPos = pos;
		li->pTailPos = pos;
	}
	if(li->pHeadPos==NullPos)
		li->pHeadPos = pos;     
    //printf("node=%x,prev=%x,pos=%x,next=%x\n",node,pos->pPrevPos,pos,pos->pNextPos);
}
ElementNode fifo_pop_node(FIFO * li)
{
	if(li==NULL)
		return NullNode;
	Position * pos = li->pHeadPos;
	if(pos==NullPos)
		return NullNode;
	ElementNode node = pos->CurrentNode;
	if(pos->pNextPos==NullPos)
	{
		li->pCurrentPos = NullPos;
		li->pHeadPos = NullPos;
		li->pTailPos = NullPos;                
	}
	else
	{
		li->pCurrentPos = (Position*)pos->pNextPos;
		li->pCurrentPos->pPrevPos = NullNode;
		li->pHeadPos = li->pCurrentPos;                
	}
	//printf("node=%x,prev=%x,pos=%x,next=%x\n",node,pos->pPrevPos,pos,pos->pNextPos);
	free(pos);
	return node;
}
ElementNode fifo_getcurrent_node(FIFO * li)
{
	if(li==NULL)
		return NullNode;
	if(li->pCurrentPos==NullPos)
		return NullNode;
	return li->pCurrentPos->CurrentNode;
}





