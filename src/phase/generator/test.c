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
int age(int age);
void main();

// Functions
char* test() {
char* bbb = (char*) malloc(strlen("hey") + 1);
if (bbb == NULL) {
    printf("Memory allocation failed in bbb");
    exit(1);
}
strcpy(bbb, "hey");
char* aaa = (char*) malloc(strlen(strcat(bbb, "!!!")) + strlen(bbb) + 1);
if (aaa == NULL) {
    printf("Memory allocation failed in aaa");
    exit(1);
}
strcpy(aaa, strcat(strcat(bbb, "!!!"), bbb));
return (aaa);
}

int age(int age) {
return (age * 10);
}

void main() {
printf("%d", age(200));
printf("%s", "Hello, World");
printf("%s", test());
}

