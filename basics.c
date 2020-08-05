#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
//online compiler password: VMoY6CY1

//maximum number of elements in the hash table
#define HASH_TABLE_SIZE 2147483647 //this is not needed anymore
#define DEBUG_ACTIVE 1 //debugging flag

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

struct deletedHistory { //contains the list of deleted indexes
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
void debugPrintTombstones(delHistory *tombs);
struct command* getCommand(char* input);
void quit();

//global variable that defines the hash table (array of pointers to stringList objects, rows start from 1)
stringList **hashTable;
int hashCapacity;

int hash(int n) { //hash function with linear hashing
	//needs to check and skip the deleted rows
	if (n > lineCapacity) {
		return 0;
	} else if (linePointers[n] == -1) {
		return 0;
	} else return linePointers[n];
}

void change(int start, int end, char** text) { //changes the text
	//function declaration without malloc is: char (*text)[1024]
	int range = end - start + 1;
	int i;

	for (i = 0; i < range; i++) { //updates linePointers array and its dimension
		if (hash(start + i) == 0) {
			//extension
			lineCapacity += 1; //lineCapacity += range;
			linePointers = realloc(linePointers, (lineCapacity + 1) * sizeof(int));

			linePointers[start + i] = linePointers[start + i - 1] + 1;

		} else if(hash(start + i) == -1) {
			linePointers[start + i] = linePointers[start + i - 1] + 1;
		}
	}
	/*
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
	*/
	int currentPos = hash(start);

	if (hash(end) > hashCapacity) { //increment size of hash table with a fast approximation, not every time it is needed
		hashTable = realloc(hashTable, (hashCapacity + range) * sizeof(stringList));
		hashCapacity += range;
	}

	for (i = 0; i < range; i++) { //every row is inserted in the hash table
		insertHashTable(currentPos, text[i]);
		start++;
		currentPos = hash(start);
	}
}

void insertHashTable(int index, char *line) { //inserts single string in the hash table
	stringList *str = (stringList*) malloc(sizeof(stringList));
	str->string = line;
	str->deletedFlag = false;
	//insert in the head of a single linked list
	if (hashTable[index] == NULL) {
		str->next = NULL;
	} else {
		str->next = hashTable[index];
	}
	hashTable[index] = str;
}


void delete(int start, int end) { //TODO: handle invalid delete addresses, which are not present in the table
	int count = 0; //counter for tombstones
	int forward = start; //counter for linePointers
	int range = end - start + 1;
	int i = start;
	while (i <= range) { //deals with useless delete calls on non-existent indexes in the hash table
		if (hash(i) == 0) {
			end = i - 1;
			range = end - start + 1;
			i = range + 1;
		} else {
			i++;
		}
	}

	stringList *str[range];
	for (i = 0; i < range; i++) {
		//allocates one deleted piece for every element to be deleted
		str[i] = (stringList*) malloc(sizeof(stringList));
	}

	delHistory *newTomb = (delHistory *) malloc(sizeof(struct deletedHistory));
	newTomb->number = range;
	newTomb->next = tombstones; //insert at the head of the linked list
	newTomb->list = (int *) malloc(newTomb->number * sizeof(int));

	i = hash(start);

	while (hashTable[i] != NULL) {
		if (i >= hash(start) && i <= hash(end) && hashTable[i]->deletedFlag == false) {
			newTomb->list[count] = i;
			str[count]->next = hashTable[i];
			str[count]->string = "";
			str[count]->deletedFlag = true;
			hashTable[i] = str[count];
			count++;
		} else if (hashTable[i]->deletedFlag == false) {
			linePointers[forward] = i;
			forward++;
		}

		i++;
	}
	while (forward <= lineCapacity && linePointers[forward] != -1) {
		//sets every number after the last one needed to -1 in the array linePointers
		linePointers[forward] = -1;
		forward++;
	}

	tombstones = newTomb; //changes the list of tombstones by accessing at its address
	/*
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
	*/
}

void debugPrintTombstones(delHistory *tombs) {
	//prints the sequence list of deleted lines in the hash table
	//one element for every delete call, and every element is a list of indexes
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
				//printf("-----------------change call\n");
			}
			change(start, end, text);
			if (DEBUG_ACTIVE) {
				//debugPrintFullTable(true);
			}

		} else if (action == 'p') {
			//print lines, used only for the online compiler
			if (DEBUG_ACTIVE) {
				printf("print sequence from %d to %d started:\n", start, end);
			}
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
				//printf("-----------------delete call %d,%d\n", start, end);
			}

			delete(start, end);
			if (DEBUG_ACTIVE) {
				//debugPrintFullTable(true);
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
	if (strcmp(input, "q\n") == 0) { //equal strings
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
	return commando; //returns the struct containing the information of the command in input
}

void quit() { //input = 'q'
	//these lines for freeing memory are actually unnecessary and don't benefit the memory usage
	free(linePointers);
	free(tombstones);
	free(hashTable);
	exit(0);
}

void debugPrintFullHashLine(int index, stringList* pointer) { //prints all the contents in a single line of a hash table
	if (pointer->next == NULL) {
		printf("%s --> null\n", pointer->string);
	} else {
		if (pointer->deletedFlag == false) {
			printf("%s --> ", pointer->string);
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
			printf("%d: ", i);
			debugPrintFullHashLine(i, hashTable[i]); //full hash line debug
			i++;
		}
	}
}

int main() {
	//initializations
	linePointers = (int*) malloc(sizeof(int)); //modify this to have just one
	linePointers[0] = 0;

	//initialization of the list of deleted lines
	tombstones = (delHistory*) malloc(sizeof(delHistory));
	tombstones->number = 0;
	tombstones->next = NULL;
	tombstones->list = NULL;

	hashCapacity = 10; //starting point for the hash table dimension
	hashTable = realloc(hashTable, hashCapacity * sizeof(stringList));

	if (DEBUG_ACTIVE) {
		//char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test10.txt"; //only for testing and debugging
		char fileName[] = "/home/simon/CS Project/TextEditor/inputTest3.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform
	} else {
		input(stdin);
	}

	return 0;
}
