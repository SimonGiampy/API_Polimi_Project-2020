#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//int hash[2147483647];
struct stringList { //single linked list for handling changes in the text
		char* string;
		bool deletedFlag;
		struct stringList* next;
};
typedef struct stringList stringList;
int hash[500000000];

int main() {
    printf("%d\n", (int) sizeof(bool));
	printf("%d\n", (int) sizeof(char*));
	printf("%d\n", (int) sizeof(stringList));

    stringList *str = (stringList*) malloc(sizeof(stringList));
	str->string = "2kjsd<gdvl<sdjkvLSro IHIoGHW<ÒOEHGW GEHIOW HFEUIh< ÒKJfv<sdjbjvDJVBN GREBIG ";
	str->deletedFlag = false;
	str->next = str;
	char string[] = "2kjsd<gdvl<sdjkvLSro IHIoGHW<ÒOEHGW GEHIOW HFEUIh< ÒKJfv<sdjbjvDJVBN GREBIG ";

	printf("%d\n", (int) sizeof(string));

    return 0;
}

