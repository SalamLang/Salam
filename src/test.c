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
extern char* test();

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
char* temp_var_1 = a;
char* temp_var_2 = int2str(500);
char* temp_var_0 = malloc(sizeof(char) * (strlen(temp_var_1) + strlen(temp_var_2)) + 1);
strcpy(temp_var_0, temp_var_1);
strcat(temp_var_0, temp_var_2);
return (temp_var_0);
}

char* main() {
printf("%s", "\n");
printf("%s", test());
}