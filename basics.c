#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
//online compiler password: VMoY6CY1

//maximum number of elements in the hash table
#define HASH_TABLE_SIZE 2147483647
#define DEBUG_ACTIVE 1

struct stringList { //single linked list for handling changes in the text
	char* string;
	bool deletedFlag;
	struct stringList* next;
};
typedef struct stringList stringList;

int* linePointers; //keeps track of every line position in the table
int lineCapacity = 0; //number of lines used in the hash table

struct command { //every input is of the form (ind1,ind2)action
	int start;
	int end;
	char command;
};

struct deletedHistory {
	struct deletedHistory *next;
	int number;
	int *list;
};
typedef struct deletedHistory delHistory;
delHistory *tombstones = NULL;

//function explicit declarations
void insertHashTable(int i, char* line);
void input(FILE *fp);
int hash(int n);
void change( int start, int end, char** text);
void delete(int start, int end);
void debugPrintFullHashLine(int index, stringList* pointer);
void debugPrintFullTable(bool showFullLineContents);
struct command* getCommand(char* input);
void quit();

//global variable that defines the hash table (array of pointers to stringList objects, rows start from 1
stringList* hashTable[HASH_TABLE_SIZE];

int hash(int n) { //hash function with linear hashing
	//needs to check and skip the deleted rows

	//insert deletion mechanism update
	if (n > lineCapacity) return 0; else return linePointers[n];
}

void change(int start, int end, char** text) { //changes the text
	//function declaration without malloc is: char (*text)[1024]
	int range = end - start + 1;
	int i = 0;

	for (i = 0; i < range; i++) {
		if (hash(start + i) == 0) {
			//extension
			lineCapacity += 1; //lineCapacity += range;
			linePointers = realloc(linePointers, (lineCapacity + 1) * sizeof(int));

			linePointers[start + i] = linePointers[start + i - 1] + 1;

		} else if(hash(start + i) == -1) {
			linePointers[start + i] = linePointers[start + i - 1] + 1;
			//lineCapacity += 1;
		} else if(hashTable[hash(start + i)]->deletedFlag == true) {
			//TODO: check all the special cases for the deleted and test longer texts
		}
	}
	if (DEBUG_ACTIVE) {
		printf("progressive:   ");
		for (i = 0; i <= lineCapacity; i++) {
			printf("%d\t|", i);
		}
		printf("\naddress index: ");
		for (i = 0; i <= lineCapacity; i++) {
			printf("%d\t|", linePointers[i]);
		}
		printf("\n");
	}

	int currentPos = hash(start);

	for (i = 0; i < range; i++) {
		insertHashTable(currentPos, text[i]); //hashing here
		start++;
		currentPos = hash(start);
	}
}

void insertHashTable(int index, char *line) { //inserts single string in the hash table
	stringList *str = (stringList*) malloc(sizeof(stringList));
	str->string = line;
	str->deletedFlag = false;

	if (hashTable[index] == NULL) {
		str->next = NULL;
	} else {
		str->next = hashTable[index];
	}
	hashTable[index] = str;
}

void debugPrintTombstones(delHistory *tombs) {
	int c = 0;
	while (tombs != NULL) {
		c++;
		printf("deleted history %d: ", c);
		for (int i = 0; i < tombs->number; i++) {
			printf("%d ", tombs->list[i]);
		}
		printf("\n");
		tombs = tombs->next;
	}
}

void delete(int start, int end) {
	int count = 0; //counter for tombstones
	int forward = start; //counter for linePointers

	stringList *str = (stringList*) malloc(sizeof(stringList));
	str->string = "";
	str->deletedFlag = true;

	delHistory *newTomb = (delHistory *) malloc(sizeof(struct deletedHistory));
	newTomb->number = end - start + 1;
	newTomb->next = tombstones; //insert at the head of the linked list
	newTomb->list = (int *) malloc(newTomb->number * sizeof(int));

	int i = hash(start);

	while (hashTable[i] != NULL) {
		if (hashTable[i]->deletedFlag == true) {
			//nothing
		} else if (i >= hash(start) && i <= hash(end)) {
			newTomb->list[count] = i;
			count++;
			str->next = hashTable[i]->next;
			hashTable[i] = str;
		} else if (hashTable[i]->deletedFlag == false) {
			linePointers[forward] = i;
			forward++;
		}

		i++;
	}
	while (forward <= lineCapacity && linePointers[forward] != -1) {
		linePointers[forward] = -1;
		forward++;
	}

	tombstones = newTomb; //changes the list of tombstones by accessing at its address

	if (DEBUG_ACTIVE) {
		printf("progressive:   ");
		for (i = 0; i <= lineCapacity; i++) {
			printf("%d\t|", i);
		}
		printf("\naddress index: ");
		for (i = 0; i <= lineCapacity; i++) {
			printf("%d\t|", linePointers[i]);
		}
		printf("\n");
		debugPrintTombstones(tombstones);
	}
}


void input(FILE *fp) { //fp is the file pointer passed from main
	if (fp == NULL) return;
	char buffer[1024 + 2]; //1024 max characters, +1 for \n, +1 for \0
	char* correctRead;

	while (!feof(fp)) { //until the input file is not finished reading (end of file)
		correctRead = fgets(buffer, 1024 + 2, fp);
		assert(correctRead != NULL);

		struct command* command = getCommand(buffer); //translates the line with the input (ind1,ind2)command in a struct containing the parameters
		int start = command->start;
		int end = command->end;
		int num = end - start + 1;
		char action = command->command;
		free(command);

		if (action == 'c') {
			//char **text = (char**) malloc(sizeof(char*) * num);
			char *text[num]; //allocates an array of num strings, one for each line
			for (int i = 0; i < num; i++) {
				correctRead = fgets(buffer, 1024 + 2, fp);
				assert(correctRead != NULL);
				text[i] = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
				strcpy(text[i], buffer);
				//text[i] = (char*) malloc(sizeof(char) * (1024 + 2));
				//correctRead = fgets(text[i], 1024 + 2, fp);
				//assert(correctRead != NULL);
			}
			correctRead = fgets(buffer, 3, fp); //reads the terminal sequence of the input text ".\n\0" -> 3 chars
			assert(correctRead != NULL);
			assert(strcmp(buffer, ".\n") == 0); //exits in case there isn't a full stop, this should never be called
			if (DEBUG_ACTIVE) {
				printf("-----------------change call\n");
			}

			change(start, end, text);
			if (DEBUG_ACTIVE) {
				debugPrintFullTable(false);
			}

		} else if (action == 'p') {
			//print lines, used only for the online compiler
			for (int i = start; i <= end; i++) {
				if (hashTable[hash(i)] != NULL) {
					fputs(hashTable[hash(i)]->string, stdout); //only first string in each list
				} else {
					fputs(".\n", stdout); //no string present in the selected line
				}

			}
		} else if (action == 'd') {
			//delete lines, import from dynamic indexes
			if (DEBUG_ACTIVE) {
				printf("-----------------delete call %d,%d\n", start, end);
			}

			delete(start, end);
			if (DEBUG_ACTIVE) {
				debugPrintFullTable(false);
			}

		} else if (action == 'u' && end == 0) {
			//undo
		} else if (action == 'r' && end == 0) {
			//redo
		}

	}
	fclose(fp);
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
	if (strcmp(input, "q") == 0) { //equal strings
		quit();
	}

	struct command *commando = (struct command*) malloc(sizeof(struct command));
	int i = 0, comma = 1;
	int start, end;
	char action;

	char *tmp = (char*) malloc(sizeof(char) * 9); //9 is the max number of digits since there are at most 2B lines
	while (input[i] != '\n') {
		if (input[i] == ',') {
			strncpy(tmp, input, i); //take substring 1 before comma char
			start = (int) strtol(tmp, NULL, 10); //converts substring in long number
			comma = i; //comma position in the string
		} else if (input[i] == 'c' || input[i] == 'd' || input[i] == 'p') {
			strncpy(tmp, input + comma  + 1, i - comma  - 1); //take substring 2 before char 'c'
			end = (int) strtol(tmp, NULL, 10); //converts substring in long number
			action = input[i];
		} else if (input[i] == 'u' || input[i] == 'r') {
			action = input[i];
			strncpy(tmp, input, i); //take substring 1 before action char
			start = (int) strtol(tmp, NULL, 10);
			end = 0; //undo and redo only take one input number
		}
		i++;
	}
	free(tmp);

	commando->start = start;
	commando->end = end;
	commando->command = action;
	return commando;
}

void quit() { //input = 'q'
	exit(0);
}

void debugPrintFullHashLine(int index, stringList* pointer) { //prints all the contents in a single line of a hash table
	printf("%d: ", index);
	if (pointer->next == NULL) {
		printf("%s --> null\n", pointer->string);
	} else {
		if (pointer->deletedFlag == false) {
			printf("%s --> \n", pointer->string);
		} else {
			printf("DELETED --> ");
		}
		debugPrintFullHashLine(index, pointer->next);
	}
}

void debugPrintFullTable(bool showFullLineContents) { //prints all the lines in the hash table from line 1 to max
	int i = 1;
	if (!showFullLineContents) { //prints only the first string for every row
		while (hashTable[i] != NULL) {
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				printf("%s", hashTable[i]->string);
			}
			i++;
		}
	} else { //prints every string for every row
		while (hashTable[i] != NULL) {
			debugPrintFullHashLine(i, hashTable[i]); //full hash line debug
			i++;
		}
	}
}

int main(int argc, char *argv[]) {
	//initializations
	linePointers = (int*) malloc(sizeof(int)); //modify this to have just one
	linePointers[0] = 0;

	//initialization of the list of deleted lines
	tombstones = (delHistory*) malloc(sizeof(delHistory));
	tombstones->number = 0;
	tombstones->next = NULL;
	tombstones->list = NULL;

	if (DEBUG_ACTIVE) {
		char fileName[] = "/home/simon/CS Project/TextEditor/inputTest2.txt"; //only for testing and debugging
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform
	} else {
		input(stdin);
	}

	return 0;
}
