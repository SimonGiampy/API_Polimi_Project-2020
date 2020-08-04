//
// Created by simon on 29/07/20.
//
#include <stdlib.h>
#include <stdio.h>

struct indexes {
	int tableIndex; //actual indexes present in the table, -1 corresponds to deleted lines
	int progressiveIndex; //from 1 until the end, cardinal ordered numbers for keeping track of the indexes
	int pointer;
};

struct deletedHistory {
	struct deletedHistory *next;
	int number;
	int *list;
};
typedef struct deletedHistory delHistory;

void printTwoLinesOnly(int from, int to);
void printTombstones(delHistory *tombstones);
void delete(int start, int end, delHistory **tombstones);
//delHistory* delete2(int start, int end, delHistory *tombstones);
//global variables
struct indexes *orderedIndexes;
int *pointers;
int capacity;

int main() {
	capacity = 30; //initial capacity
	orderedIndexes = (struct indexes*) malloc(capacity * sizeof(struct indexes));
	pointers = (int*) malloc(capacity * sizeof(int));

	//initialization
	struct indexes temp;
	for (int i = 1; i <= capacity; i++) {
		temp.progressiveIndex = i;
		temp.tableIndex = i;
		temp.pointer = i;
		orderedIndexes[i - 1] = temp;
		pointers[i-1] = i;
	}

	delHistory *tombstones = (delHistory*) malloc(sizeof(delHistory));
	tombstones->number = 0;
	tombstones->next = NULL;
	tombstones->list = NULL;

	delete(6, 8, &tombstones);
	delete(13, 14, &tombstones);
	delete(5, 7, &tombstones);
	delete(7, 8, &tombstones);
	delete(7, 9 , &tombstones);
	delete(1, 2, &tombstones);

	printTwoLinesOnly(1, capacity);
	printTombstones(tombstones);
}


void delete(int start, int end, delHistory **tombstones) {
	int jumps = 0;
	int count = 0;
	int forward = 1;

	delHistory *newTomb = (delHistory*) malloc(sizeof(struct deletedHistory));
	newTomb->number = end - start + 1;
	newTomb->next = *tombstones; //insert at the head of the linked list
	newTomb->list = (int*) malloc(newTomb->number * sizeof(int));

	for (int i = 1; i <= capacity; i++) {
		if (orderedIndexes[i - 1].tableIndex == -1) {
			jumps++;
		} else if (i <= orderedIndexes[end - 1].pointer && i >= orderedIndexes[start - 1].pointer) {
			orderedIndexes[i - 1].tableIndex = -1;
			jumps++;
			newTomb->list[count] = i - 1;
			count++;
		} else if (orderedIndexes[i - 1].tableIndex != -1) {
			orderedIndexes[i - 1].tableIndex = i - jumps;
			orderedIndexes[forward - 1].pointer = i;
			forward++;
		}
	}
	for (; forward <= capacity; forward++) {
		orderedIndexes[forward - 1].pointer = -1;
	}

	*tombstones = newTomb; //changes the list of tombstones by accessing at its address
	//free(newTomb);
	/*
	int range = end - start + 1;
	int i = start;
	for (; i <= capacity - range; i++) {
		pointers[i-1] = i + range;
	}
	*/
}

void printTombstones(delHistory *tombstones) {
	int c = 0;
	while (tombstones != NULL) {
		c++;
		printf("deleted history %d: ", c);
		for (int i = 0; i < tombstones->number; i++) {
			printf("%d ", tombstones->list[i]);
		}
		printf("\n");
		tombstones = tombstones->next;
	}
}

void printTwoLinesOnly(int from, int to) {
	printf("progressive: ");
	for (int i = from; i <= to; i++) {
		printf("%d\t|", orderedIndexes[i - 1].progressiveIndex);
	}
	printf("\nreal table:  ");
	for (int i = from; i <= to; i++) {
		printf("%d\t|", orderedIndexes[i - 1].tableIndex);
	}
	printf("\npoint index: ");
	for (int i = from; i <= to; i++) {
		printf("%d\t|", orderedIndexes[i - 1].pointer);
	}
	/*
	printf("\narray point: ");
	for (int i = from; i <= to; i++) {
		printf("%d\t", pointers[i - 1]);
	}
	 */
	printf("\n");
}
