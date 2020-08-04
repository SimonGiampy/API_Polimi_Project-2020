#include <stdlib.h>
#include <stdio.h>

struct interval {
	int low; //lower bound of a single interval
	int high; //upper bound of a single interval
	int cumulativeJumps; //cumulative number of holes present before the single interval considered
};
typedef struct interval interval;

//this struct can be optimized by converting it in an auto-balancing binary search tree
struct listIntervals {
	struct listIntervals *next;
	interval obj;
};
typedef struct listIntervals listIntervals;

void updateHoles(int newLow, int newHigh, listIntervals* list);
void printRanges(int limit, int vector[100]);
void updateVector(listIntervals* list, int vector[100]);

void printList(listIntervals* list);

void shiftIntervals(struct listIntervals *stack, int value);

int main() {
	//this section is used for debugging and calculating the result after deleting several rows
	int vector[100]; //100 is the max number of elements to test
	for (int i = 0; i < 100; i++) { //initialization is redundant
		vector[i] = i;
	}

	listIntervals* list = (listIntervals*) malloc(sizeof(listIntervals));
	/*
	interval* hole = (interval*) malloc(sizeof(interval));
	//first element is <0,0> by default
	hole->low = 0;
	hole->high = 0;
	hole->cumulativeJumps = 0;

	list->obj = *hole;
	list->next = NULL;
	free(hole);
*/

	updateHoles(20, 21, list);
	updateHoles(3, 5, list);
	updateHoles(30, 35, list);
	updateHoles(10, 15, list);

	//updateHoles(3, 6, list);
	//updateHoles(3, 5, list);

	updateVector(list, vector);
	printRanges(50, vector);
	printList(list);

	return 0;
}

void printList(listIntervals *list) {
	while (list != NULL) {
		//format is <low, high, cumulativeJumps>
		printf("<%d, %d, %d>    ", list->obj.low, list->obj.high, list->obj.cumulativeJumps);
		list = list->next;
	}
	printf("\n");
}

void updateHoles(int newLow, int newHigh, listIntervals* list) {
	interval *hole = (interval*) malloc(sizeof(interval)); //creates new interval to insert in the list
	hole->low = newLow;
	hole->high = newHigh;
	hole->cumulativeJumps = newHigh - newLow + 1;

	listIntervals *newPiece = (listIntervals*) malloc(sizeof(listIntervals));
	newPiece->obj = *hole;
	newPiece->next = NULL;
	free(hole);

	while (list != NULL) {
		if (list->next != NULL) {

			if (newLow > list->obj.high && newHigh < list->next->obj.low) { //basic insertion and right shift of elements
				newPiece->obj.low = newLow + (list->obj.high - list->obj.low);
				newPiece->obj.high = newHigh + (list->obj.high - list->obj.low);
				newPiece->obj.cumulativeJumps += list->obj.cumulativeJumps;
				newPiece->next = list->next;
				list->next = newPiece;

				shiftIntervals(newPiece->next, newPiece->obj.high - newPiece->obj.low + 1);
				return;
			}
		} else { //last piece to be inserted in the list
			if (newLow > list->obj.high) {
				newPiece->obj.low = newLow + (list->obj.high - list->obj.low);
				newPiece->obj.high = newHigh + (list->obj.high - list->obj.low);
				newPiece->obj.cumulativeJumps += list->obj.cumulativeJumps;
				list->next = newPiece;
				return;
			} /*else if (newLow == list->obj.high + 1) {
				//bugged piece
				free(newPiece);
				newHigh += (list->obj.high - list->obj.low);
				list->obj.high = newHigh;
				return;
			}*/
		}

		newLow += (list->obj.high - list->obj.low + 1);
		newHigh += (list->obj.high - list->obj.low + 1);
		newPiece->obj.low = newLow;
		newPiece->obj.high = newHigh;
		list = list->next; //list advancement
	}

	//list is empty at the moment
	list = newPiece;
}

void shiftIntervals(listIntervals *stack, int value) {
	while (stack != NULL) {
		stack->obj.low += value;
		stack->obj.high += value;
		stack->obj.cumulativeJumps += value;
		stack = stack->next;
	}
}

void updateVector(listIntervals* list, int vector[100]) {
	//this function build up the vector using as reference the array of intervals
	int jumps = 0; //number of places to jump, starts from -1 to take account of i=0 jumped by default
	//i iterates through the vector and assigning the values

	for (int i = 1; i < 100; i++) { //iteration through vector
		 if (list != NULL) {
			 if (i >= list->obj.low && i <= list->obj.high) { //elements between an interval
				 vector[i] = -1;
				 jumps++;
				 if (i == list->obj.high) { //increments pos after last element in the present interval
					 list = list->next;
				 }
			 } else if (i < list->obj.low) { //before the first element of the next interval
				 vector[i] = i - jumps;
			 } else {
				 vector[i] = i; //the elements before the first interval
			 }
		 } else {
			 vector[i] = i - jumps;
		 }
	}
}

void printRanges(int limit, int vector[100]) { //prints the content of vector from 1 to limit, used for debugging
	for (int i = 1; i < limit; i++) {
		printf("index %d -> value = %d\n", i, vector[i]);
	}
	printf("end\n");
}