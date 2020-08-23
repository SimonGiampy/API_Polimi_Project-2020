// Created by simon on 23/08/20
// online compiler password: VMoY6CY1
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define DEBUG_ACTIVE 1
#define INPUT_FROM_USER 0

struct command { //every input is of the form (ind1,ind2)action
	int start;
	int end;
	char command;
};

//append-only array of strings, which contains every string inserted in input
char** appendText;
int lastPosition;

//array of indexes that contains the index position of a certain string in the append only array
int* currentPositions;
//the first index 0 is used to calculate the number of lines inserted at the end (writeonly trick)
int lastRow;

//array of struct commands for keeping track of the history of changes
struct command *history;
int currentTime; //position in the array of the present state of the text, decreased by undo
int pastActions; //number of changes and deletes done in total until a certain point in time, only increased

//array of pointers to arrays of integer positions
struct state {
	int lastRow; //the real index of the last row in the array which contains a valid number
	int lastPosition; //the index of the last string saved in the text state
	int size; //dimension of the array of indexes (probably useless parameter)
	int* indexes; //pointer to the base pointer of the array of indexes
};
typedef struct state states;
states* textStates; //array of text states


struct command* getCommand(char* input);
void debugAppendText(void );
void debugHistory(void );
void debugCurrentPositions(void );
void input(FILE* fp);
void quit();

void change(int start, int end, char **text);
void printText(int from, int to);
void delete(int start, int end);
void deleteByMovingMemory(int start, int end);
void saveState(void );
void debugTextStates();

void input(FILE *fp) { //fp is the file pointer passed from main
	if (fp == NULL) return;
	char buffer[1024 + 1]; //1024 max characters, +1 for \0
	char* correctRead;

	while (!feof(fp)) { //until the input file is not finished reading (end of file)
		correctRead = fgets(buffer, 1024 + 1, fp);
		assert(correctRead != NULL);

		struct command* command = getCommand(buffer); //translates the line with the input (ind1,ind2)command in a struct
		int start = command->start;
		int end = command->end;
		int num = end - start + 1;
		char action = command->command;

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

			change(start, end, text);
			//saves in memory the command given in input
			history[currentTime] = *command;
			currentTime++;
			pastActions++;

			free(command);

		} else if (action == 'p') { //outputs the text in the form asked by the project specification
			if (start == 0 && end == 0) {
				fputs(".\n", stdout);
			} else {
				printText(start, end);
			}
		} else if (action == 'd') {
			if (!(start == 0 && end == 0)) {
				delete(start, end);
				//deleteByMovingMemory(start, end); //alternative method, test if it's faster
			}
			//saves in memory the command given in input
			history[currentTime] = *command;
			currentTime++;
			pastActions++;

			free(command);
		}

	}
	fclose(fp);
}

void printText(int from, int to) {
	int points = 0;
	if (to > lastRow) { //calculates how many empty lines to print afterwards
		points = to - lastRow;
		to = lastRow;
	}
	if (from == 0) {
		fputs(".\n", stdout);
		from = 1;
	}

	int index = currentPositions[from]; //first index row
	for (int i = from; i <= to; ) {
		fputs(appendText[index], stdout);
		i++;
		index = currentPositions[i];
	}
	for (int i = 0; i < points; i++) { //prints empty non-existent or deleted lines
		fputs(".\n", stdout);
	}
}

void change(int start, int end, char **text) {
	//TODO: optimize memory usage for change calls that append strings to the end
	//TODO: handle changes in the middle of the text after some writes at the end of the text
	int j = lastPosition;
	int from = start;
	for (int i = 0; i < end - start + 1; i++, j++, from++) {
		appendText[j] = text[i]; //appends the new text given in input
		currentPositions[from] = j; //writes the correct index in the array of current indexes
	}
	lastPosition = j; //updates last entry index of appendText array

	if (lastRow < end) { //updates the last index representation
		lastRow = end;
	}
	saveState();
}

//first method for deleting lines is based on copying the successive indexes of the next lines
void delete(int start, int end) {
	//TODO: handle invalid delete calls
	if (start > lastRow) {
		//add to the history a delete call that does nothing
		return;
	}

	if (end > lastRow) { //out of bounds, corrects endpoint
		end = lastRow;
	}
	if (start == 0) { //invalid starting point
		start = 1;
	}
	int skip = end + 1;
	lastRow -= end - start + 1; //decrease the number of rows present in the text after a deletion

	//O(n) time complexity, where n = number of lines present in the text
	for (int i = start; i <= lastRow; i++, skip++) {
		currentPositions[i] = currentPositions[skip];
	}
	//the rows after the last effective one contain numbers without meaning, so be careful not to access them
	saveState();
}

//use memmove function to slide up the numbers and cover the deleted elements
void deleteByMovingMemory(int start, int end) {
	//this method is probably not faster than the other one
	//TODO: add the clauses for handling invalid delete calls
	int* dest = currentPositions + start;
	int* src = currentPositions + end + 1;
	int number = lastRow - end;

	memmove(dest, src, sizeof(int) * number);
	lastRow -= end - start + 1; //decrease the number of rows present in the text
}

void saveState(void ) { //save current state in the struct state array
	//allocate space for a copy of the current state
	int size = lastRow + 1;

	int* indexesArray = (int*) malloc(sizeof(int) * size);
	memcpy(indexesArray, currentPositions, sizeof(int) * size); //copies the current state of the memory in the array

	textStates[currentTime].lastRow = lastRow;
	textStates[currentTime].lastPosition = lastPosition;
	textStates[currentTime].indexes = indexesArray;
}

void debugAppendText() {
	printf("showing the strings in memory from 0 to %d:\n", lastPosition);
	for (int i = 0; i < lastPosition; i++) {
		printf("%d : %s", i, appendText[i]);
	}
}

void debugCurrentPositions() {
	printf("showing the indexes related to the strings from 0 to %d:\n", lastRow);
	for (int i = 1; i <= lastRow; i++) {
		printf("%d : %d\n", i, currentPositions[i]);
	}
}

void debugHistory() {
	printf("showing the history of changes from 0 to %d (current = %d) :\n", pastActions, currentTime);
	for (int i = 0; i <= pastActions; i++) {
		printf("%c : %d,%d\n", history[i].command, history[i].start, history[i].end);
	}
}

void debugTextStates() {
	printf("showing the text states using index numbers:\n");
	int* array;
	for (int i = 0; i <= pastActions; i++) { //for every saved state in the history
		printf("lastRow = %d, lastPosition = %d : ", textStates[i].lastRow, textStates[i].lastPosition);
		array = textStates[i].indexes;
		for (int j = 1; j <= textStates[i].lastRow; j++) { //for every index saved in the array of indexes
			printf("%d\t", array[j]);
		}
		printf("\n");
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
	if (INPUT_FROM_USER == 1) {
		exit(0);
	}
}

int main() {
	//initializations
	int staticLinesCapacity = 100000000; //100 million lines seems to be more than enough according to write-only task
	//static capacity is not augmented to avoid dynamic reallocation, in order to save time to allocate space memory
	appendText = (char**) malloc(staticLinesCapacity * sizeof(char*));
	lastPosition = 0;

	currentPositions = (int*) malloc(staticLinesCapacity * sizeof(int));
	currentPositions[0] = 0;
	lastRow = 0;

	int staticHistoryCapacity = 10000000; //10 million change and delete calls to be stored in the history of changes
	//this static capacity should be big enough to contain every change made
	history = (struct command*) malloc(staticHistoryCapacity * sizeof(struct command));
	currentTime = 0; //current position in the history

	textStates = (states*) malloc(staticHistoryCapacity * sizeof(states));
	pastActions = 0;

	clock_t start, end;
	if (DEBUG_ACTIVE) {
		start = clock();
	}

	if (INPUT_FROM_USER == 1) {
		input(stdin);
	} else {
		//char fileName[] = "/home/simon/CS Project/Write_Only_2_input.txt";
		//char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test1000.txt"; //only for testing and debugging
		char fileName[] = "/home/simon/CS Project/TextEditor/inputTest.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform

	}

	if (DEBUG_ACTIVE) {
		end = clock();
		double timeTaken = (double) (end - start) / CLOCKS_PER_SEC;
		printf("time taken is %.4f\n", timeTaken);
		debugHistory();
		debugTextStates();
	}

	return 0;
}