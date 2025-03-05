// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// Extended functions
extern char* int2str(int c);

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
printf("%s", int2str(412));
return (a);
}

char* main() {
printf("%s", "\n");
}