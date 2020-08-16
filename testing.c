#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "red-black-tree-working.c"

struct stringList { //single linked list for handling changes in the text
		char* string;
		bool deletedFlag;
		struct stringList* next;
};
typedef struct stringList stringList;

struct collection {
	char** stack;
	unsigned short currentPosition;
	unsigned short size;
	bool deletedFlag;
	bool isAllocated;
};

struct collection **table;



int main() {

	//initialization of quicklookups structure
	quickLookupsHead = (struct quickLookupsHead*) malloc(sizeof(struct quickLookupsHead));
	quickLookupsHead->capacity = 4; //just for testing a limited amount of intervals
	quickLookupsHead->occupied = 0;

	insertInterval(1, 5);
	insertInterval(11, 13);
	insertInterval(8, 9);
	insertInterval(7, 7);
	insertInterval(27, 29);
	printInorderTrasversal(tree);
	printf("\n");

	//lookups test 1 2 3
	printf("lookup 1 = %d\n", quickLookup(1));

	//this section tests for the dynamic decrease of an integer array of 20 integers, down to 10 integers
	/*
	int startSize = 20;
	int *memory = (int*) malloc(sizeof(int) * startSize);
	for (int i = 0; i < startSize; i++) { //initialization
		memory[i] = i + 1;
	}
	int newSize = 10; //shrink size

	int* tmp = malloc(newSize * sizeof(int)); //shrinks the memory down to 10 bytes
	//memmove copies the first 10 * 4 bytes of data (10 integers) from the pointer memory to the temporary pointer tmp
	memmove(tmp, memory, newSize * sizeof(int));
	free(memory); //frees old block of memory
	memory = tmp; //copied the pointers so the temporary one can be discarded
	free(tmp); //avoids memory leaks

	for (int i = 0; i < newSize; i++) {
		printf("%d ", memory[i]); //outputs the copied 10 integers
	}
*/
    return 0;
}

