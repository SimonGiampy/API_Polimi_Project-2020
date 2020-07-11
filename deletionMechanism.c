#include <stdlib.h>
#include <stdio.h>

struct interval {
	int low; //lower bound of a single interval
	int high; //upper bound of a single interval
	int cumulativeJumps; //cumulative number of holes present before the single interval considered
};
typedef struct interval interval;

struct listIntervals {
	struct listIntervals *next;
	interval obj;
};
typedef struct listIntervals listIntervals;

void updateHoles(int newLow, int newHigh, listIntervals* list, int* num);
void printRanges(int limit, int vector[100]);
void updateVector(listIntervals* list, int num, int vector[100]);

int main() {
	//this section is used for debugging and calculating the result after deleting several rows
	int vector[100]; //100 is the max number of elements to test
	for (int i = 0; i < 100; i++) { //initialization is redundant
		vector[i] = i;
	}

	listIntervals* list = (listIntervals*) malloc(sizeof(listIntervals));

	interval* hole = (interval*) malloc(sizeof(interval));
	//first element is <0,0> by default
	hole->low = 0;
	hole->high = 0;
	hole->cumulativeJumps = 0;

	list->obj = *hole;
	list->next = NULL;
	free(hole);

	int* num = (int*) malloc(sizeof(int)); //number of elements in the list of intervals
	*num = 1;

	updateHoles(3, 5, list, num);
	updateVector(list, 2, vector);
	printRanges(20, vector);

	return 0;
}

void updateHoles(int newLow, int newHigh, listIntervals* list, int* num) {
	interval *hole = (interval*) malloc(sizeof(interval)); //creates new interval to insert in the list
	hole->low = newLow;
	hole->high = newHigh;

	listIntervals *newPiece = (listIntervals*) malloc(sizeof(listIntervals));
	newPiece->obj = *hole;
	newPiece->next = NULL;
	free(hole);

	int i = 0;
	while (i < *num) {


		if (newLow > list->obj.high) { //new interval is after the last one
			newPiece->obj.cumulativeJumps = list->obj.cumulativeJumps + (newHigh - newLow + 1);
			list->next = newPiece;
			//list->next->obj.cumulativeJumps = list->obj.cumulativeJumps + (newHigh - newLow + 1);
		}

		i++;
	}
	(*num)++; //increments the number of elements in the array of intervals
}

void updateVector(listIntervals* list, int num, int vector[100]) {
	//this function build up the vector using as reference the array of intervals
	int pos = 0; //iteration through holes list
	int jumps = -1; //number of places to jump, starts from -1 to take account of i=0 jumped by default
	//i iterates through the vector and assigning the values
	listIntervals* copy = list;
	for (int i = 0; i < 100; i++) { //iteration through vector
		if (pos == num) { //elements after the last interval
			vector[i] = i - jumps;
		} else { //when the array of intervals isn't finished yet
			if (i >= copy->obj.low && i <= copy->obj.high) { //elements between an interval
				vector[i] = -1;
				jumps++;
				if (i == copy->obj.high) { //increments pos after last element in the present interval
					pos++;
					copy = copy->next;
				}
			} else if (i < copy->obj.low) { //before the first element of the next interval
				vector[i] = i - jumps;
			} else {
				vector[i] = i; //the elements before the first interval
			}
		}
	}
	free(copy);
}

void printRanges(int limit, int vector[100]) { //prints the content of vector from 1 to limit, used for debugging
	for (int i = 1; i < limit; i++) {
		printf("index %d -> value = %d\n", i, vector[i]);
	}
	printf("end\n");
}