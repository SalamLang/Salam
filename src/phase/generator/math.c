#include <math.h>
#include <stdlib.h>
#include <stdio.h>

float math_sin(float value) {
    return sin(value);
}

float math_cos(float value) {
    return cos(value);
}

float math_tan(float value) {
    return tan(value);
}

char* int2str(int value) {
    char* str = (char*)malloc(12);
    if (str) {
        sprintf(str, "%d", value);
    }
    return str;
}

char* float2str(float value) {
    char* str = (char*)malloc(20);
    if (str) {
        sprintf(str, "%f", value);
    }
    return str;
}

int str2int(const char* str) {
    return atoi(str);
}

float str2float(const char* str) {
    return atof(str);
}

char* math_creator = "Max";
