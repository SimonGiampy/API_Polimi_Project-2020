#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
//online compiler password: VMoY6CY1

//maximum number of elements in the hash table
#define DEBUG_ACTIVE 1 //debugging flag
//TODO: TimeForAChange has timeout, so an optimization for the change mechanism must be found
//the problem shouldn't be in the delete function
//TODO: fix memory leaks by testing it with big text test input files

struct stringList { //single linked list for handling changes in the text
	char* string;
	bool deletedFlag;
	struct stringList* next;
};
typedef struct stringList stringList;

struct collection {
	char** stack;
	unsigned short currentPosition;
	unsigned short size;
	bool deletedFlag;
};
typedef struct collection collection;

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
void debugPrintTombstones(delHistory *tombs);
void debugPrintNew(bool showFullLineContents);
struct command* getCommand(char* input);
void quit();

//global variable that defines the hash table (array of pointers to stringList objects, rows start from 1)
collection **hashTable = NULL;
int hashCapacity;

int hash(int n) { //hash function with linear hashing
	//needs to check and skip the deleted rows
	//NOTE: if a negative number n is passed as input, it means that the input test file has at its end a 'q' without '\n'
	if (n > lineCapacity || linePointers[n] == -1) {
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

	int hashedEnd = hash(end);
	if (hashedEnd > hashCapacity) { //increment size of hash table with a fast approximation, not every time it is needed
		hashTable = realloc(hashTable, (hashedEnd + 1) * sizeof(collection));

		//extreme bug fix
		for (int j = hashCapacity; j <= hashedEnd; j++) {
			hashTable[j] = NULL;
		}
		hashCapacity = hashedEnd + 1;
	}

	int currentPos = hash(start);
	for (i = 0; i < range; i++) { //every row is inserted in the hash table
		insertHashTable(currentPos, text[i]);
		start++;
		currentPos = hash(start);
	}
}

void insertHashTable(int index, char *line) { //inserts single string in the hash table
	if (hashTable[index] != NULL) { //for some absurd reason this is executed when it must not
		//increments stack size by one, and inserts a new element at the end
		hashTable[index]->currentPosition += 1;

		char** stack = realloc(hashTable[index]->stack, (hashTable[index]->size + 1) * sizeof(collection));
		stack[hashTable[index]->currentPosition] = line;
		hashTable[index]->size += 1;
		hashTable[index]->stack = stack;
		hashTable[index]->deletedFlag = false;
	} else if (hashTable[index] == NULL) {
		//creation of a new element in the table
		collection *coll = (collection*) malloc(sizeof(collection));
		coll->deletedFlag = false;
		coll->size = 1;
		coll->currentPosition = 0;

		char** stack = (char**) malloc(sizeof(char*));
		stack[0] = line;
		coll->stack = stack;
		hashTable[index] = coll;
		//TODO: fix memory leak for certain input test files, like the n.3
	}
}


void delete(int start, int end) {
	int count = 0; //counter for tombstones
	int forward = start; //counter for linePointers
	int range = end - start + 1;
	int i = start;

	/*if (hash(end) == 0) { //faster optimization
		end = lineCapacity - 1;
		range = end - start + 1;
	}*/

	while (i <= range) { //deals with useless delete calls on non-existent indexes in the hash table
		if (hash(i) == 0) {
			end = i - 1;
			range = end - start + 1;
			i = range + 1;
		} else {
			i++;
		}
	}


	delHistory *newTomb = (delHistory *) malloc(sizeof(struct deletedHistory));
	newTomb->number = range;
	newTomb->next = tombstones; //insert at the head of the linked list
	newTomb->list = (int *) malloc(newTomb->number * sizeof(int));

	i = hash(start);

	while (hashTable[i] != NULL) { //fix here for the new mechanism
		if (i >= hash(start) && i <= hash(end) && hashTable[i]->deletedFlag == false) {
			newTomb->list[count] = i;
			hashTable[i]->deletedFlag = true;
			count++;
		} else if (hashTable[i]->deletedFlag == false) {
			linePointers[forward] = i;
			forward++;
		}

		i++;
	}
	//TODO: implement the mechanism for resizing the array linePointers, so all the -1s at the end get trimmed out
	while (forward <= lineCapacity && linePointers[forward] != -1) {
		//sets every number after the last one needed to -1 in the array linePointers
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
		//assert(end >= start && start >= 0 && end >=0);

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
				printf("change sequence from %d to %d started:\n", start, end);
			}
			change(start, end, text);
			//for (int i = 0; i < num; i++) {
				//free(text[i]); //segfaults always
			//}

			if (DEBUG_ACTIVE) {
				//debugPrintNew(true); //new method to test the correctness of the program
			}

		} else if (action == 'p') {
			//print lines, used only for the online compiler
			if (DEBUG_ACTIVE) {
				printf("print sequence from %d to %d started:\n", start, end);
			}
			collection *coll ;
			for (int i = start; i <= end; i++) {
				coll = hashTable[hash(i)];
				if (coll != NULL) {
					fputs(coll->stack[coll->currentPosition], stdout); //only first string in each list
				} else {
					fputs(".\n", stdout); //no string present in the selected line
				}
			}
			//free(coll);
		} else if (action == 'd') {
			//delete lines, import from dynamic indexes
			if (DEBUG_ACTIVE) {
				printf("-----------------delete call %d,%d\n", start, end);
			}

			delete(start, end);
			if (DEBUG_ACTIVE) {
				debugPrintNew(true);
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
	debugPrintNew(true);
	free(linePointers);
	free(tombstones);
	free(hashTable);
	exit(0);
}

void debugPrintNew(bool showFullLineContents) {
	int i = 1;
	if (!showFullLineContents) { //only shows the first string for every line
		while (hashTable[i] != NULL) {
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				unsigned short pos = hashTable[i]->currentPosition;
				printf("%s", hashTable[i]->stack[pos]);
			}
			i++;
		}
	} else { //shows all the strings in the stack for every line in the table
		while (hashTable[i] != NULL) {
			printf("%d: ", i);
			unsigned short pos = hashTable[i]->currentPosition;
			for (int count = 0; count < hashTable[i]->size; count++) {
				printf("%s -> ", hashTable[i]->stack[pos]);
				pos--;
			}
			i++;
			printf("\n");
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

	hashCapacity = 1; //starting point for the hash table dimension
	hashTable = realloc(hashTable, hashCapacity * sizeof(collection));

	if (DEBUG_ACTIVE) {
		//char fileName[] = "/home/simon/CS Project/Write_Only_1_input.txt";
		//char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test10.txt"; //only for testing and debugging
		char fileName[] = "/home/simon/CS Project/TextEditor/inputTest3.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform

	} else {
		input(stdin);
	}


	return 0;
}
