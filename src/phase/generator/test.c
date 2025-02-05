// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// External libraries
// Import math

// Extended functions
extern float math_sin(float value);
extern char* int2str(int value);

// Sign functions
char* _int2str(int a);
char* main();

// Functions
char* _int2str(int a) {
return ("110");
}

char* main() {
printf("%s", _int2str(0));
printf("%g", math_sin(90));
char* temp_var_0 = malloc(sizeof(char) * (1 + 1) + 1);
strcpy(temp_var_0, "1");
strcat(temp_var_0, "!");
printf("%s", temp_var_0);
char* temp_var_2 = int2str(110);
char* temp_var_1 = malloc(sizeof(char) * (1 + strlen(temp_var_2)) + 1);
strcpy(temp_var_1, "2");
strcat(temp_var_1, temp_var_2);
printf("%s", temp_var_1);
}

