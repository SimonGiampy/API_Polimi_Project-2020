#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "red-black-tree-working.c"


int main() {

	//initialization of quicklookups structure
	quickLookupsHead = (struct quickLookupsHead*) malloc(sizeof(struct quickLookupsHead));
	quickLookupsHead->capacity = 5; //just for testing a limited amount of intervals
	quickLookupsHead->occupied = 0;

	//int array[] = {436, 747, 33, 391, 561, 598, 864, 931, 176, 618, 462, 608, 65, 342, 716, 741, 712, 755, 788, 886, 213, 322};
	//add 400 564

	int array[] = {436, 747, 33, 391, 561, 598, 864, 931, 176, 618, 462, 608, 65, 342, 716, 741, 712, 755, 788, 886, 213, 322};
	for (int i = 0; i < 22; i +=2) {
		printf("added interval <%d,%d> \n", lookup(array[i]), lookup(array[i+1]));
		insertInterval(lookup(array[i]), lookup(array[i+1]));
		printInorderTrasversal(tree);
		printf("\n");
	}
	printInorderTrasversal(tree);
	printf("\n");
	printf("lookup 400 = %d\n", lookup(400));


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

