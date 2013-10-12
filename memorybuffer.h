#define __ELEMENT_NODE__
// 这个头文件必须在fifo前，才能使用ElementNode，否则fifo会调用内部自定义的结构体。
typedef struct 
{
	char * lpBuffer;
	int nLoc;
	int nSize;
	int bUsed;
}MemoryBuffer,*ElementNode;

#include "fifo.h"

#ifndef __MEMORY_H__
#define __MEMORY_H__

// 分配MemoryBuffer内存，释放MemoryBuffer内存
MemoryBuffer * memory_new_data(int size);
void memory_delete_data(MemoryBuffer * mem);
//打印数据
void memory_print_data(FIFO * li);
// 得到内部数据的个数，不是fifo中ElementNode的个数。
int memory_get_data_size(FIFO * li);
// 把li中的数据复制到output中，output的尺寸最大是outsize个字节。
int memory_output_data(char * output,int outsize,FIFO * li);
// 把input中的数据复制到li中，input中的数据个数是insize个字节。
int memory_input_data(const char * input,int insize,FIFO * li);















#endif
