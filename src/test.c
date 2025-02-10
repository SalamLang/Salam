// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// Extended functions
extern char* int2str(int value);

// Sign functions
char* _int2str(int a);
char* main();

// Functions
char* _int2str(int a) {
return ("110");
}

char* main() {
char* bbb = (char*) malloc(strlen("hey") + 1);
if (bbb == NULL) {
    printf("Memory allocation failed in bbb\n");
    exit(1);
}
strcpy(bbb, "hey");
char* aaa = (char*) malloc(strlen(bbb) + strlen("!!!") + strlen(bbb) + strlen("??? END") + strlen(int2str(33)) + 1);
if (aaa == NULL) {
    printf("Memory allocation failed in aaa\n");
    exit(1);
}
strcat(aaa, bbb);
strcat(aaa, "!!!");
strcat(aaa, bbb);
strcat(aaa, "??? END");
strcat(aaa, int2str(33));
printf("%s", aaa);
return (aaa);
printf("%s", aaa);
}

