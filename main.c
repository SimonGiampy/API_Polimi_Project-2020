#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//password: VMoY6CY1

//maximum number of elements in the hash table
#define HASH_TABLE_SIZE 2147483647

//boolean declaration
//typedef enum {false, true} bool;

struct stringList { //single linked list for handling changes in the text
    char* string;
    struct stringList* next;
};
typedef struct stringList stringList;

struct command { //every input is of the form (ind1,ind2)action
    unsigned int start;
    unsigned int end;
    char command;
};

//function explicit declarations
void insertHashTable(int i, char* line);
void input(FILE *fp);
unsigned int hash(unsigned int n);
void change(unsigned int start, unsigned int end, char** text);
void printFullHashLine(int i, stringList* pointer);
void printFullTable(int max);
struct command* getCommand(char* input);

void quit();

//global variable that defines the hash table (array of pointers to stringList objects, rows start from 1
stringList* hashTable[HASH_TABLE_SIZE];

unsigned int hash(unsigned int n) { //hash function with linear hashing
    unsigned int value = n; //sample
    //needs to check and skip the deleted rows
    return value;
}

void change(unsigned int start, unsigned int end, char** text) { //changes the text
    //declaration without malloc is: char (*text)[1024]
    unsigned int pos = start;
    for (unsigned int i = 0; i < end - start + 1; i++) {
        insertHashTable((int) pos, text[i]);
        pos++;
    }
}

void insertHashTable(int i, char *line) { //inserts single string in the hash table
    stringList *str = (stringList*) malloc(sizeof(stringList));
    str->string = line;
    unsigned int index = hash(i); //calculates the correct position for the line
    if (hashTable[index] == NULL) {
        str->next = NULL;
    } else {
        str->next = hashTable[index];
    }
    hashTable[index] = str;
}

void input(FILE *fp) { //fp is the file pointer passed from main
	if (fp == NULL) return;
	char buffer[1024 + 2]; //1024 max characters, +1 for \n, +1 for \0

	while (!feof(fp)) { //until the input file is not finished reading (end of file)
		fgets(buffer, 1024 + 2, fp);

		struct command* command = getCommand(buffer); //translates the line with the input (ind1,ind2)command in a struct containing the parameters
		unsigned int start = command->start;
		unsigned int end = command->end;
		unsigned int num = end - start + 1;
		char action = command->command;
		//this below works only for change command 'c', since it reads the text

		char **text = (char**) malloc(sizeof(char*) * num); //allocates an array of num strings, one for each line
		for (int i = 0; i < num; i++) {
			text[i] = (char*) malloc(sizeof(char) * (1024 + 2));
			fgets(text[i], 1024 + 2, fp);
		}
		fgets(buffer, 3, fp); //reads the terminal sequence of the input text ".\n\0" -> 3 chars
		if (buffer[0] != '.') {
			exit(1); //exits in case there isn't a full stop, this should never be called
		}
		if (action == 'c') { //add the actions for the other commands
			change(start, end, text);
		}
	}
	fclose(fp);
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
    if (strcmp(input, "q") == 0) { //equal strings
		quit();
    }

	struct command* commando = (struct command*) malloc(sizeof(struct command));
    int i = 0, comma = 1;
    long start, end;
    char action;

    char *tmp = (char*) malloc(sizeof(char) * 9); //9 is the max number of digits since there are at most 2B lines
    while (input[i] != '\n') {
        if (input[i] == ',') {
            memcpy(tmp, input, i); //take substring 1 before comma char
            start = strtol(tmp, NULL, 10); //converts substring in long number
            comma = i; //comma position in the string
        } else if (input[i] == 'c' || input[i] == 'd') {
            memcpy(tmp, input + comma + 1, i - comma - 1); //take substring 2 before char 'c'
            end = strtol(tmp, NULL, 10); //converts substring in long number
        	action = input[i];
        }
        i++;
    }
    free(tmp);

    commando->start = (unsigned int) start;
    commando->end = (unsigned int) end;
    commando->command = action;
    return commando;
}

void quit() { //input = 'q'
	exit(0);
}

void printFullHashLine(int i, stringList* pointer) { //prints all the contents in a single line of a hash table
    printf("%d: ", i);
    if (pointer->next == NULL) {
        printf("%s -> null\n", pointer->string);
    } else {
        printf("%s --> ", pointer->string);
        printFullHashLine(i, pointer->next);
    }
}

void printFullTable(int max) { //prints all the lines in the hash table from line 1 to max
    for (int i = 1; i <= max; i++) {
        if (hashTable[i] == NULL) {
            printf("nothing\n");
        } else {
        	printFullHashLine(i, hashTable[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    char fileName[] = "/home/simon/CS Project/TextEditor/inputTest.txt";
    FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
    input(fp); //fp must be stdin when submitting the code on the platform

    printFullTable(14);
    return 0;
}
