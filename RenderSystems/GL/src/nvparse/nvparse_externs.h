#ifndef NVPARSE_EXTERNS_H
#define NVPARSE_EXTERNS_H

extern nvparse_errors errors;
extern int line_number;
extern char * myin;

#ifdef _WIN32
#include <windows.h>
#endif
#include <glad/glad.h>

#endif
