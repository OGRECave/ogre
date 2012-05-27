#ifndef NVPARSE_H
#define NVPARSE_H

#define NVPARSE 1

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif



void nvparse(const char * input_string, int argc = 0,...);
char** nvparse_get_errors();
char** nvparse_print_errors(FILE *fp);
const int* nvparse_get_info(const char* input_string, int* pcount);
#ifdef __cplusplus
}
#endif

#endif
