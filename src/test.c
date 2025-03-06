// Extern Includes
#include <stdio.h>

// Extern Functions
extern char* int2str(int c);

// Extern Variables
extern int age;

// Sign functions
int main();

// Functions
int main() {
char* name = (char*) malloc(strlen("Max") + 1);
if (name == NULL) {
    printf("Memory allocation failed in name\n");
    exit(1);
}
strcpy(name, "Max");
char* temp_var_1 = name;
char* temp_var_0 = malloc(sizeof(char) * (4 + strlen2(temp_var_1)) + 1);
strcpy(temp_var_0, "Hey ");
strcat(temp_var_0, temp_var_1);
char* temp_var_2 = malloc(sizeof(char) * (14 + 1) + 1);
strcpy(temp_var_2, temp_var_0);
strcat(temp_var_2, "!");
printf("%s", temp_var_2);
return ("fgdfg");
}