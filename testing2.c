#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

short getOffset(int x);

int main() {
	//ingenious way of converting input string to 2 integers numbers while avoiding errors
	char string[] = "45756u";
	char* endPtr;
	int start = (int) strtol(string, &endPtr, 10);
	int end = 0;
	if (*endPtr == ',') {
		end = (int) strtol(endPtr + 1, &endPtr, 10);
	}
	char action = *endPtr;

	//short offset = strlen(string) - strlen(endPtr) + 1; //approach 1
	//short offset = getOffset(start) + 1;

	//int end = strtol(endPtr + 1, &endPtr, 10);
	printf("%d\t%d\t%c\t%s\n", start, end, action, endPtr);

}

short getOffset(int x) {
	// this is either a fun exercise in optimization
	// or it's extremely premature optimization.
	if(x < 100000) {
		if(x >= 1000) {
			if(x >= 10000) return 5;
			return 4;
		} else {
			if(x >= 100) return 3;
			if(x >= 10) return 2;
			return 1;
		}
	} else {
		if(x >= 10000000) {
			if(x >= 1000000000) return 10;
			if(x >= 100000000) return 9;
			return 8;
		}
		if(x >= 1000000) return 7;
		return 6;
	}
}