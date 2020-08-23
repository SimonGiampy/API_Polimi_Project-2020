//
// Created by simon on 23/08/20.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
	int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	memmove(array + 1, array + 3, 7 * sizeof(int));

	for (int i = 0; i < 10; i++) {
		printf("%d\t", array[i]);
	}
	printf("\n");
	return 0;
}
