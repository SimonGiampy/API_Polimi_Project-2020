//
// Created by simon on 23/08/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct command { //every input is of the form (ind1,ind2)action
	int start;
	int end;
	char command;
};

//append-only array of strings, which contains every string inserted in input
char** appendText;
int lastPosition;

//array of indexes that contains the index position of a certain string in the append only array
int** positions;

//array of struct commands for keeping track of the history of changes

//array of pointers to arrays of integer positions


struct command* getCommand(char* input);
void debugAppendText(void );
void input(FILE* fp);
void quit();

void change(int start, int end, char **text);

void input(FILE *fp) { //fp is the file pointer passed from main
	if (fp == NULL) return;
	char buffer[1024 + 1]; //1024 max characters, +1 for \0
	char* correctRead;

	while (!feof(fp)) { //until the input file is not finished reading (end of file)
		correctRead = fgets(buffer, 1024 + 1, fp);
		assert(correctRead != NULL);

		struct command* command = getCommand(buffer); //translates the line with the input (ind1,ind2)command in a struct containing the parameters
		int start = command->start;
		int end = command->end;
		int num = end - start + 1;
		char action = command->command;
		free(command);

		if (action == 'c') {
			char *text[num]; //allocates an array of num strings, one for each line
			for (int i = 0; i < num; i++) {
				correctRead = fgets(buffer, 1024 + 1, fp);
				assert(correctRead != NULL);
				text[i] = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
				strcpy(text[i], buffer);
			}
			correctRead = fgets(buffer, 3, fp); //reads the terminal sequence of the input text ".\n\0" -> 3 chars
			assert(correctRead != NULL);
			//assert(strcmp(buffer, ".\n") == 0); //exits in case there isn't a full stop, this should never be called

			change(start, end, text);

		} else if (action == 'p') {

		} else if (action == 'd') {
			//delete(start, end);
		}

	}
	fclose(fp);
}

void change(int start, int end, char **text) {
	//TODO: extend append text dimension dynamically or with a sufficient starting memory size
	//TODO: check the maximum number of rows inserted in write-only in order to avoid dynamic reallocation
	int j = lastPosition;
	for (int i = 0; i < end - start + 1; i++, j++) {
		appendText[j] = text[i];
	}
	lastPosition = j;
}

void debugAppendText() {
	printf("showing the strings in memory from 0 to %d\n", lastPosition);
	for (int i = 0; i < lastPosition; i++) {
		printf("%d : %s", i, appendText[i]);
	}
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
	if (strcmp(input, "q\n") == 0 || strcmp(input, "q") == 0) { //equal strings
		quit();
	}

	struct command *commando = (struct command*) malloc(sizeof(struct command));
	//new better error-free version
	char* endPtr;
	int start = (int) strtol(input, &endPtr, 10);
	int end = 0;
	if (*endPtr == ',') {
		end = (int) strtol(endPtr + 1, &endPtr, 10);
	}
	char action = *endPtr;

	commando->start = start;
	commando->end = end;
	commando->command = action;
	return commando; //returns the struct containing the information of the command in input
}

void quit() { //input = 'q'
	//these lines for freeing memory are actually unnecessary and don't benefit the memory usage
	exit(0);
}

int main() {
	//initializations
	appendText = (char**) malloc(1000 * sizeof(char*));
	lastPosition = 0;

	char fileName[] = "/home/simon/CS Project/TextEditor/inputTest.txt";
	FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
	input(fp); //fp must be stdin when submitting the code on the platform

	return 0;
}