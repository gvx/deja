#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "opcodes.h"
#include "stack.h"
#include "value.h"
#include "header.h"
//#include "stdlib.h"
//#include "env.h"

int main(int argc, char *argv[])
{
	Stack astack = *newstack();
	printf("%d\n", astack.size);
	int i;
	for (i=7; i<14; i+=2)
	{
		//V v = malloc(sizeof(Value)); 
		//v = int_to_value(i);
		V v = int_to_value(i);
		push(&astack, v);
		printf("%d, %f, %f\n", astack.size, v->data.number, astack.head->data->data.number);
	}
	printf("%f\n", astack.head->next->next->data->data.number);
	printf("%d\n", astack.size);
	while(size_of(&astack))
	{
		V v2 = pop(&astack);
		printf("%d, %f\n", astack.size, v2->data.number);
	}
	printf("%d\n", astack.size);
	V n = newlist();
	printf("\\ %d\n", n->refs);
	add_ref(n);
	printf("\\ %d\n", n->refs);
	clear_ref(n);
	printf("\\ %d\n", n->refs);
	clear_ref(n);
	printf("1: ");
	scanf("%d", &i);
	#define UUU 100
	#define VVV 100000
	V* k = malloc(UUU * sizeof(Value));
	char *wrd = malloc(VVV);
	memset(wrd, '\x42', VVV);
//	int i;
	for (i = 0; i < UUU; i++)
	{
		k[i] = a_to_value(wrd);
	}
	free(wrd);
	printf("2: ");
	scanf("%d", &i);
	for (i = 0; i < UUU; i++)
	{
		clear_ref(k[i]);
	}
	free(k);
	printf("3: ");
	scanf("%d", &i);
	printf("3b: ");
	scanf("%d", &i);
}
