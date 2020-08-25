// Created by Simon Giampy on 23/08/20
// online compiler password: VMoY6CY1
// Simone Giampà
// matricola: 909739 ; codice persona: 10659184
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

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
void input(FILE* fp);
void change(int start, int end, char **text);
void appendOnly(int start, int end, char **text);
void printText(int from, int to);
void delete(int start, int end);
void timeMachine(int number);
void newTimeline(void);
void saveState(void );
void saveStateAppend(void );
void restoreAppendedStates(void );

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

				text[i] = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
				strcpy(text[i], buffer);
			}
			correctRead = fgets(buffer, 3, fp); //reads the terminal sequence of the input text ".\n\0" -> 3 chars

			int timeVariation = pastActions - undoStack; //new temporal position in the history

			if (timeVariation != currentTime) { //if something gets changed
				timeMachine(timeVariation); //shifts the indicator for the time position
			}
			currentTime++; //increments position in time
			if (start == lastRow + 1 && WRITE_ONLY_SUBTASKS) {
				appendOnly(start, end, text); //special case to handle write only test case
				pastActions++;
			} else {
				if (WRITE_ONLY_SUBTASKS) {
					WRITE_ONLY_SUBTASKS = false;
					restoreAppendedStates();
				}

				newTimeline(); //alters the time line, since a new action canceled the undos and redos
				pastActions++;
				change(start, end, text);
			}

			free(command);

		} else if (action == 'p') { //outputs the text in the form asked by the project specification

			int timeVariation = pastActions - undoStack;

			if (timeVariation != currentTime) { //if something gets changed
				timeMachine(timeVariation); //shifts the indicator for the time position
			}

			if (start == 0 && end == 0) {
				fputs(".\n", stdout);
			} else {
				printText(start, end);
			}
			free(command);
		} else if (action == 'd') {

			if (WRITE_ONLY_SUBTASKS) {
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}
			int timeVariation = pastActions - undoStack;

			if (timeVariation != currentTime) { //if something gets changed
				timeMachine(timeVariation);
			}
			currentTime++;
			newTimeline(); //alters the time line since the delete cancels the commands executed in the future
			pastActions++;
			if (!(start == 0 && end == 0)) {
				delete(start, end);
			}

			free(command);
		} else if (action == 'q') {
			free(command);
			break;
		} else if (action == 'u') {

			if (WRITE_ONLY_SUBTASKS) { //restores the previous states which haven't been saved yet
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}

			//calculation of undo commands
			if (pastActions < undoStack + start) {
				undoStack = pastActions;
			} else {
				undoStack += start;
			}
			free(command);
		} else if (action == 'r') {

			if (WRITE_ONLY_SUBTASKS) { //restores the previous states which haven't been saved yet
				WRITE_ONLY_SUBTASKS = false;
				restoreAppendedStates();
			}
			//calculation of redo commands
			if (undoStack < start) {
				undoStack = 0;
			} else {
				undoStack -= start;
			}
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
	currentTime = number;
	lastPosition = textStates[currentTime].lastEntry;
	lastRow = textStates[currentTime].rows;

	int size = textStates[currentTime].rows + 1;
	memcpy(currentPositions, textStates[currentTime].indexesArray, sizeof(int) * size);

}

void newTimeline(void) {
	if (currentTime < pastActions + 1) {

		for (int i = currentTime; i <= pastActions; i++) {
			free(textStates[i].indexesArray);
			textStates[i].lastEntry = 0;
			textStates[i].rows = 0;
		}

		pastActions = currentTime - 1;
		undoStack = 0;

	} //else the timeline is already reset
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
	currentTime = 0; //current position in the history

	textStates = (states*) malloc(staticHistoryCapacity * sizeof(states));
	textStates[0].indexesArray = currentPositions; //empty text to start with
	textStates[0].rows = 0;
	textStates[0].lastEntry = 0;

	pastActions = 0; //number of actions in the current timeline
	undoStack = 0; //number of undos to apply, only positive values accepted

	WRITE_ONLY_SUBTASKS = true; //the first call must be from 1 onwards

	input(stdin);

	return 0;
}