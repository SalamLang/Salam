#ifndef PUBAPI_H
#define PUBAPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t pubapi_Add(int32_t a, int32_t b);
const char* pubapi_Greeting(void);

#ifdef __cplusplus
}
#endif

#endif
