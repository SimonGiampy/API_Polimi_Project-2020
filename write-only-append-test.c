//
// Created by simon on 24/08/20.
//

//this script serves as purpose to test if the write-only task contains only append text queries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int* getCommand(char* input);

int main() {
	char buffer[1025];
	int *command;
	int start = 0, end = 0;
	int oldEnd = 0;
	char action;
	char* check;

	while (strcmp(buffer, "q\n") != 0) {
		command = getCommand(buffer);
		start = command[0];
		end = command[1];
		action = (char) command[2];

		if (action == 'c') {
			if (start == oldEnd + 1) { //append only queries
				oldEnd = end;
			} else {
				exit(4);
			}

			for (int i = 0; i < end - start + 1; i++) {
				check = fgets(buffer, 1025, stdin); //read lines
			}
			check = fgets(buffer, 1025, stdin); //point
		}
		check = fgets(buffer, 1025, stdin); //new command read
	}

	return 0;
}

int* getCommand(char* input) {
	char* endPtr;
	int start = (int) strtol(input, &endPtr, 10);
	int end = 0;
	if (*endPtr == ',') {
		end = (int) strtol(endPtr + 1, &endPtr, 10);
	}
	char action = *endPtr;

	int *vec = (int*) malloc(12);
	vec[0] = start;
	vec[1] = end;
	vec[2] = action;
	return vec;
}