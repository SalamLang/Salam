// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// Sign functions
char* test();
char* main();

// Functions
char* test() {
char* a = (char*) malloc(strlen("Hey ") + 1);
if (a == NULL) {
    printf("Memory allocation failed in a\n");
    exit(1);
}
strcpy(a, "Hey ");
return (a);
}

char* main() {
printf("%s", "\n");
}