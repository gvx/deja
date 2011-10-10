#ifndef STACK_DEF
#define STACK_DEF

#include "value.h"

typedef struct Node
{
	struct Value *data;
	struct Node *next;
} Node;

typedef struct
{
	int size;
	Node *head;
} Stack;

Stack* newstack();
void push(Stack*, V);
V pop(Stack*);
int stack_size(Stack*);

#endif
