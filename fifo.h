#ifndef __ELEMENT_NODE__
#define __ELEMENT_NODE__
// 在使用FIFO以前，需要定义ElementNode，以便代替这个定义。
typedef struct
{
	int loc;
	int value;
}* ElementNode;
void memory_delete_data(ElementNode node);
#endif

//stdlib.h包含malloc，free，printf在stdio.h中。

#ifndef __FIFO_H__
#define __FIFO_H__
typedef struct
{
	ElementNode CurrentNode;
	void * pPrevPos;
	void * pNextPos;
} Position;
typedef struct {
	Position * pHeadPos;  //the first pos
	Position * pTailPos;  //the last pos
	Position * pCurrentPos; //the current pos
}FIFO;
#define NullPos NULL
#define NullNode NULL

// 内存函数，分别创建fifo，删除fifo和删除fifo中的数据，但是不删除fifo本身。
FIFO * fifo_new_list();
void fifo_delete_list(FIFO * li);
void fifo_release_data(FIFO * li);

// 不能单独使用fifo_get_next，以免找不对ElementNode
ElementNode fifo_get_head(FIFO * li);
ElementNode fifo_get_next(FIFO * li);
ElementNode fifo_get_last(FIFO * li);

// 增加一个数据到li的尾部。
void fifo_push_node(ElementNode node,FIFO * li);
// 把li的第一个数据弹出li，以便使用。
ElementNode fifo_pop_node(FIFO * li);
// 获得fifo当前的ElementNode
ElementNode fifo_getcurrent_node(FIFO * li);




#endif


