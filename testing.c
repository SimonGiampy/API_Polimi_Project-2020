#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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


char *sendNudes();

int main() {
	printf("size of struct = %d\n", (int) sizeof(struct collection));
	/*
	table = (struct collection**) malloc(2 * sizeof(struct collection));

	struct collection *coll = (struct collection*) malloc(sizeof(struct collection));
	coll->size = 1;
	coll->deletedFlag = false;
	coll->currentPosition = 0;

	char** array = (char**) malloc(1 * sizeof(char*));

	char* line = sendNudes();
	array[0] = line;
	coll->stack = array;

	for (int i = 0; i < 1; i++) {
		printf("%s\t", coll->stack[i]);
	}
	printf("\n");

	table[0] = coll;

	struct collection *coll2 = (struct collection*) malloc(sizeof(struct collection));
	coll2->size = 4;
	coll2->deletedFlag = false;
	coll2->currentPosition = 0;

	char** array2 = (char**) malloc(3 * sizeof(char*));
	array2[0] = "fdz<reyyrte";
	array2[1] = "gfxhxfgxjhtrxjg";
	array2[2] = "sdfhxh";
	array2[3] = "iorioeririorir";

	coll2->stack = array2;

	for (int i = 0; i < 4; i++) {
		printf("%s\t", coll2->stack[i]);
	}

	table[1] = coll2;
	*/

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

char *sendNudes() {
	return "lamadonna";
}

