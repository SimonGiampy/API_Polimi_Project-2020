#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "red-black-tree-working.c" //this avoids having a file longer than a thousand lines
//online compiler password: VMoY6CY1

//maximum number of elements in the hash table
#define DEBUG_ACTIVE 0 //debugging flag

struct stringList { //single linked list for handling changes in the text
	char* string;
	struct stringList* next;
};
typedef struct stringList stringList;

struct collection {
	stringList *stack;
	unsigned short currentPosition;
	//TODO: an idea could be to add a string pointer, so the access to the current string for printing is immediate (costs 8 bytes)
	bool deletedFlag;
};
typedef struct collection collection;

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
collection **hashTable;
int hashCapacity; //number of rows which have been allocated
int lastRow; //number of the last line that contains some text
delHistory *tombstones = NULL;

int main() {
	//initialization of the list of deleted lines

	tombstones = (delHistory *) malloc(sizeof(delHistory));
	tombstones->number = 0;
	tombstones->next = NULL;
	tombstones->list = NULL;

	//initialization of quicklookups structure
	quickLookupsHead = (struct quickLookupsHead*) malloc(sizeof(struct quickLookupsHead));
	quickLookupsHead->capacity = 15; //just for testing a limited amount of intervals
	quickLookupsHead->occupied = 0;

	hashCapacity = 1000; //starting point for the hash table dimension
	hashTable = malloc(hashCapacity * sizeof(collection));
	lastRow = 0;

	if (!DEBUG_ACTIVE) {
		//char fileName[] = "/home/simon/CS Project/Write_Only_1_input.txt";
		//char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test1000.txt"; //only for testing and debugging
		char fileName[] = "/home/simon/CS Project/TextEditor/inputTest6.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform
	} else {
		input(stdin);
	}

	return 0;
}

int hash(int n) { //hash function with linear hashing
	return quickLookup(n);
}

void change(int start, int end, char** text) { //changes the text
	//function declaration without malloc is: char (*text)[1024]
	int range = end - start + 1;
	int i;

	//increment size of hash table with a fast approximation, not every time it is needed
	if (hash(end) > hashCapacity) {
		hashTable = realloc(hashTable, (hashCapacity + range * 10) * sizeof(collection));
		for (int j = hashCapacity; j <= hashCapacity + range * 10; j++) {
			hashTable[j] = NULL;
		}
		hashCapacity += range * 10;
	}
	if (end > lastRow) {
		lastRow = end;
	}

	int currentPos = hash(start);
	for (i = 0; i < range; i++) { //every row is inserted in the hash table
		insertHashTable(currentPos, text[i]);
		start++;
		currentPos = hash(start);
	}

}

void insertHashTable(int index, char *line) { //inserts single string in the hash table
	stringList *str = (stringList*) malloc(sizeof(stringList));
	//currently it only handles head-insertions
	str->string = line;

	if (hashTable[index] == NULL) {
		collection *coll = (collection*) malloc(sizeof(collection));
		coll->deletedFlag = false;
		coll->currentPosition = 1;
		coll->stack = str;
		str->next = NULL;
		hashTable[index] = coll;
	} else {
		str->next = hashTable[index]->stack;
		hashTable[index]->stack = str;
	}

}

void delete(int start, int end) {
	//TODO: make sure the interval is derived correctly

	if (end > lastRow) { //deals with useless delete calls on non-existent indexes in the hash table
		end = lastRow; //but doesn't take into account the rows in the middle of the selected ones
	}
	int range = end - start + 1;

	delHistory *newTomb = (delHistory*) malloc(sizeof(struct deletedHistory));
	newTomb->next = tombstones; //insert at the head of the linked list
	newTomb->list = (int *) malloc(range * sizeof(int)); //allocates more than the necessary

	int count = 0; //counter for tombstones
	int forward = start; //counter for finding the correct lines
	int i = hash(start);

	while (hashTable[i] != NULL && count < range) { //fix here for the new mechanism
		newTomb->list[count] = i; //memorization of deleted lines from the hash table
		count++;
		hashTable[i]->deletedFlag = true;
		forward++; //next element to delete
		i = hash(forward); //hashing every element to find the correct ones
	}
	//could be added a realloc to shrink the array and reduce the number of elements to the ones strictly necessary
	//but I don't do this since it may take some time to be executed numerous times
	newTomb->number = count; //added line for keeping track of number of deleted elements
	tombstones = newTomb; //changes the list of tombstones by accessing at its address

	if (DEBUG_ACTIVE) {
		debugPrintTombstones(tombstones);
	}

	int hashStart = tombstones->list[0], hashEnd = tombstones->list[count - 1];
	insertInterval(hashStart,hashEnd);
	if (DEBUG_ACTIVE) printf("added interval <%d,%d> \n", hashStart, hashEnd);
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
	char buffer[1024 + 1]; //1024 max characters, +1 for \n, +1 for \0
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
			if (DEBUG_ACTIVE) {
				printf("-----------------change call %d,%d \n", start, end);
			}
			change(start, end, text);

			if (DEBUG_ACTIVE) {
				debugPrintFullTable(false);
			}

		} else if (action == 'p') {
			//print lines, used only for the online compiler
			if (DEBUG_ACTIVE) {
				printf("print sequence from %d to %d started:\n", start, end);
			}
			stringList *str;
			for (int i = start; i <= end; i++) {
				if (hashTable[hash(i)] != NULL) {
					str = hashTable[hash(i)]->stack;
					//goes forward until it finds the current string
					//can be optimized by storing the current selected string in the collection struct table
					for (int j = 1; j < hashTable[hash(i)]->currentPosition; j++) {
						str = str->next;
					}
					fputs(str->string, stdout); //only first string in each list
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
			//TODO: implement the function for the fusion of undos and redos in a single command
			//undo
		} else if (action == 'r' && end == 0) {
			//redo
		}

	}
	fclose(fp);
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
	if (strcmp(input, "q\n") == 0 || strcmp(input, "q") == 0) { //equal strings
		quit();
	}
	//TODO: implement faster read using the function fread, which only stops at EOF, with the help of strtok
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
	exit(0);
}

void debugPrintFullHashLine(int index, stringList* pointer) { //prints all the contents in a single line of a hash table
	if (pointer->next == NULL) {
		printf("%s --> null\n", pointer->string);
	} else {
		printf("%s --> ", pointer->string);
		debugPrintFullHashLine(index, pointer->next);
	}
}


void debugPrintFullTable(bool showFullLineContents) { //prints all the lines in the hash table from line 1 to max
	int i = 1;
	if (showFullLineContents == false) { //prints only the first string for every row
		while (hashTable[i] != NULL) {
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				printf("%s", hashTable[i]->stack->string);
			}
			i++;
		}
	} else { //prints every string for every row
		while (hashTable[i] != NULL) {
			printf("%d: ", i);
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				debugPrintFullHashLine(i, hashTable[i]->stack); //full hash line debug
			}
			i++;
		}
	}
}
