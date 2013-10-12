#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memorybuffer.h"

// memory opt:
MemoryBuffer * memory_new_data(int size)
{
	MemoryBuffer * mem = (MemoryBuffer*)malloc(sizeof(MemoryBuffer));
	if(mem==NULL)
		return NULL;
	mem->lpBuffer = (char*)malloc(size+1);
	if(mem->lpBuffer==NULL)
	{
		free(mem);
		return NULL;
	}
	mem->lpBuffer[size] = '\0';
	mem->nLoc = 0;
	mem->nSize = size;
	mem->bUsed = 1;
	return mem;
}
void memory_delete_data(MemoryBuffer * mem)
{
	if(mem==NULL)
		return;
	if(mem->lpBuffer)
		free(mem->lpBuffer);
	free(mem);
}
//
void memory_print_data(FIFO * li)
{
	MemoryBuffer * mem = fifo_get_head(li);
	while(mem)
	{
		printf("[size:%d,loc:%d]--  %s\n",mem->nSize,mem->nLoc,mem->lpBuffer);
		mem = fifo_get_next(li);
	}	
}
int memory_get_data_size(FIFO * li)
{
	int sum = 0;
	MemoryBuffer * mem = fifo_get_head(li);
	while(mem)
	{
		sum += mem->nSize;
		mem = fifo_get_next(li);
	}
	
	return sum;
}
int memory_output_data(char * output,int outsize,FIFO * li)
{
	int sum = 0;
	int len;
	MemoryBuffer * mem = fifo_get_head(li);
	while(mem)
	{
		len = mem->nSize-mem->nLoc;
		if(len<=outsize)
		{
			memcpy(output+sum,mem->lpBuffer+mem->nLoc,len);
			sum += len;
			fifo_pop_node(li);
			memory_delete_data(mem);
			mem = fifo_getcurrent_node(li);
		}
		else
		{
			memcpy(output+sum,mem->lpBuffer+mem->nLoc,outsize);
			mem->nLoc += outsize;
			sum += outsize;
			break;
		}
	}
	return sum;
}
int memory_input_data(const char * input,int insize,FIFO * li)
{
	MemoryBuffer * mem = memory_new_data(insize);
	if(mem==NULL)
		return 0;
	fifo_push_node(mem,li);
	memcpy(mem->lpBuffer,input,insize);
	
	return insize;
}




