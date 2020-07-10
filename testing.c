#include <stdio.h>
#include <stdlib.h>

void passing(char **text, int num) {
    for (int i = 0; i < num; i++) {
        printf("%d: %s\n", i, text[i]);
    }
}

int main(int argc, char *argv[]) {
    int num = 7; //number of rows
    char **text = (char**) malloc(sizeof(char*) * num);
    text[0] = "hfjf";
    text[1] = "dfaadf";
    text[2] = "sg<rde<";
    text[3] = "ewTWTWwe";
    text[4] = "dfaadf";
    text[5] = "sg<rde<";
    text[6] = "ewTWTWwe";
    passing(text, num);

    return 0;
}

