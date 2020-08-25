// Created by simon on 23/08/20
// online compiler password: VMoY6CY1
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define DEBUG_ACTIVE 0
#define INPUT_FROM_USER 1

struct command { //every input is of the form (ind1,ind2)action
	int start;
	int end;
	char command;
};

//append-only array of strings, which contains every string inserted in input
char** appendText;
int lastPosition; //the index of the last string inserted at the end of the array of strings

//array of indexesArray that contains the index position of a certain string in the append only array
int* currentPositions;
int lastRow; //the index of the last effective row present in the text

//array of struct commands for keeping track of the history of changes
struct command *history;
int currentTime; //position in the array of the present state of the text, decreased by undo
int pastActions; //number of changes and deletes done in total until a certain point in time
int undoStack; //quantifies the number of undo commands that the program must execute
//undoStack varies from 0 upwards and decreases when an undo is called

//this flag identifies if the task being executed is write-only, so it optimizes memory usage
bool WRITE_ONLY_SUBTASKS;

//array of pointers to arrays of integer positions
struct state {
	int rows; //the real index of the last row in the array which contains a valid number
	//rows also indicates how many lines are memorized in a certain state
	int lastEntry; //the index of the last string saved in the text state
	int* indexesArray; //pointer to the base pointer of the array of indexesArray
};
typedef struct state states;
states* textStates; //array of text states


struct command* getCommand(char* input);
void debugAppendText(void );
void debugHistory(void );
void debugCurrentPositions(void );
void input(FILE* fp);

void change(int start, int end, char **text);
void appendOnly(int start, int end, char **text);
void printText(int from, int to);
void delete(int start, int end);
void timeMachine(int number);
void newTimeline(void);
void deleteByMovingMemory(int start, int end);
void saveState(void );
void saveStateAppend(void );
void restoreAppendedStates(void );
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
		//TODO: check correctness of the update mechanism for the variables currentTime and pastActions
		//TODO: the error is situated in the increase / decrease of the undo stack in relation to current time
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

			if (DEBUG_ACTIVE) {
				printf("--------------------------------------change call (%d,%d):\n", start, end);
			}
			int timeVariation = pastActions - undoStack;
			if (DEBUG_ACTIVE) {
				printf("time travel: current = %d, new time = %d, past = %d\n", currentTime, timeVariation, pastActions);
			}
			if (timeVariation != currentTime) { //if something changed
				timeMachine(timeVariation);
			}
			currentTime++;
			if (start == lastRow + 1 && WRITE_ONLY_SUBTASKS) {
				appendOnly(start, end, text); //special case to handle write only test case
				pastActions++;
			} else {
				if (WRITE_ONLY_SUBTASKS) {
					WRITE_ONLY_SUBTASKS = false;
					restoreAppendedStates();
				}

				newTimeline();
				pastActions++;
				change(start, end, text);
			}
			//saves in memory the command given in input
			history[currentTime] = *command;
			//currentTime++;
			//pastActions++;

			free(command);

			if (DEBUG_ACTIVE) {
				debugTextStates();
			}

		} else if (action == 'p') { //outputs the text in the form asked by the project specification
			if (DEBUG_ACTIVE) {
				printf("--------------------------------------print call (%d,%d):\n", start, end);
			}

			int timeVariation = pastActions - undoStack;
			if (DEBUG_ACTIVE) {
				printf("time travel: current = %d, new time = %d, past = %d\n", currentTime, timeVariation, pastActions);
			}
			if (timeVariation != currentTime) { //if something changed
				timeMachine(timeVariation);
			}

			if (start == 0 && end == 0) {
				fputs(".\n", stdout);
			} else {
				printText(start, end);
			}
		} else if (action == 'd') {
			if (DEBUG_ACTIVE) {
				printf("--------------------------------------delete call (%d,%d):\n", start, end);
			}

			if (WRITE_ONLY_SUBTASKS) {
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}
			int timeVariation = pastActions - undoStack;
			if (DEBUG_ACTIVE) {
				printf("time travel: current = %d, new time = %d, past = %d\n", currentTime, timeVariation, pastActions);
			}
			if (timeVariation != currentTime) { //if something changed
				timeMachine(timeVariation);
				//TODO: handle deletions of empty text from current time = 0 and 1
			}
			currentTime++;
			newTimeline();
			pastActions++;
			if (!(start == 0 && end == 0)) {
				delete(start, end);
			}
			if (DEBUG_ACTIVE) {
				printf("after delete \n");
				debugTextStates();
			}
			//saves in memory the command given in input
			history[currentTime] = *command;
			//currentTime++;
			//pastActions++;

			free(command);
		} else if (action == 'q') {
			free(command);
			break;
		} else if (action == 'u') {
			if (DEBUG_ACTIVE) {
				printf("-------------------------------------- undo call %d:\n", start);
			}
			if (WRITE_ONLY_SUBTASKS) {
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}

			//calculation of undo commands
			if (pastActions < undoStack + start) {
				undoStack = pastActions;
			} else {
				undoStack += start;
			}

			if (DEBUG_ACTIVE) {
				printf("undo stack = %d\n", undoStack);
				debugTextStates();
			}
		} else if (action == 'r') {
			if (DEBUG_ACTIVE) {
				printf("-------------------------------------- redo call %d:\n", start);
			}
			if (WRITE_ONLY_SUBTASKS) {
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}
			//calculation of redo commands
			if (undoStack < start) {
				undoStack = 0;
			} else {
				undoStack -= start;
			}
			if (DEBUG_ACTIVE) {
				printf("undo stack = %d\n", undoStack);
				debugTextStates();
			}
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
	int j = lastPosition;
	int from = start;
	for (int i = 0; i < end - start + 1; i++, j++, from++) {
		appendText[j] = text[i]; //appends the new text given in input
		currentPositions[from] = j; //writes the correct index in the array of current indexesArray
	}
	lastPosition = j; //updates last entry index of appendText array

	if (lastRow < end) { //updates the last index representation
		lastRow = end;
	}
	saveState();
}

void appendOnly(int start, int end, char** text) {
	int j = lastPosition;
	int from = start;
	for (int i = 0; i < end - start + 1; i++, j++, from++) {
		appendText[j] = text[i]; //appends the new text given in input
		currentPositions[from] = j;
	}
	lastPosition = j; //updates last entry index of appendText array
	lastRow = end; //unnecessary line
	saveStateAppend();
}

//first method for deleting lines is based on copying the successive indexesArray of the next lines
void delete(int start, int end) {
	if (start > lastRow) {
		//add to the history a delete call that does nothing
		saveState();
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
	//TODO: huge memory leak by using this function
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
	int* indexes = (int*) malloc(sizeof(int) * size);
	memcpy(indexes, currentPositions, sizeof(int) * size); //copies the current state of the memory in the array

	textStates[currentTime].rows = lastRow;
	textStates[currentTime].lastEntry = lastPosition;
	textStates[currentTime].indexesArray = indexes;
}

//only for handling write only append text queries
void saveStateAppend(void) {
	textStates[currentTime].rows = 0;
	textStates[currentTime].lastEntry = lastPosition;
	textStates[currentTime].indexesArray = NULL;
}

void restoreAppendedStates(void ) {
	if (DEBUG_ACTIVE) {
		printf("restoring %d states: \n", currentTime);
	}
	int* indexes[currentTime];
	int last;
	for (int i = 1; i <= currentTime; i++) {
		last = textStates[i].lastEntry;
		textStates[i].rows = last;
		indexes[i-1] = (int*) malloc(sizeof(int) * (last + 1));
		memcpy(indexes[i-1], currentPositions, sizeof(int) * (last + 1));
		textStates[i].indexesArray = indexes[i-1];
	}
}


//redoes commands and sets the new state
void timeMachine(int number) {
	//number defines the new current time so it gets updated (can go forward or backwards)
	//if (currentTime != number) {
	currentTime = number;
	lastPosition = textStates[currentTime].lastEntry;
	lastRow = textStates[currentTime].rows;

	int size = textStates[currentTime].rows + 1;
	memcpy(currentPositions, textStates[currentTime].indexesArray, sizeof(int) * size);

	//}
}

void newTimeline(void) {
	if (currentTime < pastActions + 1) {
		//lastPosition = textStates[currentTime].lastEntry;
		//lastRow = textStates[currentTime].rows;

		for (int i = currentTime; i <= pastActions; i++) {
			free(textStates[i].indexesArray);
			textStates[i].lastEntry = 0;
			textStates[i].rows = 0;
			//reset history as well ?
		}
		//currentTime += 1;

		pastActions = currentTime - 1;

		undoStack = 0;

		if (DEBUG_ACTIVE) {
			debugTextStates();
			printf("timeline changed (current = %d, past = %d)\n", currentTime, pastActions);
		}
	} //else the timeline is already reset
}

void debugAppendText() {
	printf("showing the strings in memory from 0 to %d:\n", lastPosition);
	for (int i = 0; i < lastPosition; i++) {
		printf("%d : %s", i, appendText[i]);
	}
}

void debugCurrentPositions() {
	printf("showing the indexesArray related to the strings from 0 to %d:\n", lastRow);
	for (int i = 1; i <= lastRow; i++) {
		printf("%d : %d\n", i, currentPositions[i]);
	}
}

void debugHistory() {
	printf("showing the history of changes from 0 to %d (current = %d) :\n", pastActions, currentTime);
	for (int i = 1; i <= pastActions; i++) {
		printf("%c : %d,%d\n", history[i].command, history[i].start, history[i].end);
	}
}

void debugTextStates() {
	printf("showing the text states using index numbers:\n");
	int* array;
	for (int i = 1; i <= pastActions; i++) { //for every saved state in the history
		printf("time = %d, \trows = %2d, \tlastEntry = %2d: \t", i, textStates[i].rows, textStates[i].lastEntry);
		array = textStates[i].indexesArray;
		for (int j = 1; j <= textStates[i].rows; j++) { //for every index saved in the array of indexesArray
			printf("%d\t", array[j]);
		}
		printf("\n");
	}
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
	struct command *commando = (struct command*) malloc(sizeof(struct command));

	if (strcmp(input, "q\n") == 0 || strcmp(input, "q") == 0) { //equal strings
		commando->command = 'q';
		commando->start = 0;
		commando->end = 0;
		return commando;
	}

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


int main() {
	//initializations of static and dynamic arrays
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
	textStates[0].indexesArray = currentPositions; //empty text to start with
	textStates[0].rows = 0;
	textStates[0].lastEntry = 0;

	pastActions = 0; //number of actions in the current timeline
	undoStack = 0; //number of undos to apply, only positive values accepted

	WRITE_ONLY_SUBTASKS = true; //the first call must be from 1 onwards

	clock_t start, end;
	if (DEBUG_ACTIVE) {
		start = clock();
	}

	if (INPUT_FROM_USER == 1) {
		input(stdin);
	} else {
		//char fileName[] = "/home/simon/CS Project/Rolling_Back_2_input.txt";
		//char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test1000.txt"; //only for testing and debugging
		char fileName[] = "/home/simon/CS Project/TextEditor/generatedTest.txt";
		//char fileName[] = "/home/simon/CS Project/TextEditor/inputTest6.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform

	}

	if (DEBUG_ACTIVE) {
		end = clock();
		double timeTaken = (double) (end - start) / CLOCKS_PER_SEC;
		printf("time taken is %.4f\n", timeTaken);
		//debugHistory();
		debugTextStates();
	}

	return 0;
}