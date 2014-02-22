#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvparse_errors.h"



nvparse_errors::nvparse_errors()
{
    num_errors = 0;
    reset();
}

nvparse_errors::~nvparse_errors()
{
    reset();
}


void nvparse_errors::reset()
{
    for(int i=0; i < num_errors; i++)
        free(elist[i]);//FIXME detail_nmap something is writing 0x2 to elist[1] blah!
    for(int j=0; j <= NVPARSE_MAX_ERRORS; j++)
        elist[j] = 0;
    num_errors = 0;
}

void nvparse_errors::set(const char * e)
{
    if(num_errors < NVPARSE_MAX_ERRORS)
        elist[num_errors++] = strdup(e);
}

void nvparse_errors::set(const char * e, int line_number)
{
    char buff[256];
    sprintf(buff, "error on line %d: %s", line_number, e);
    if(num_errors < NVPARSE_MAX_ERRORS)
        elist[num_errors++] = strdup(buff);
}

char** nvparse_errors::get_errors()
{
    return elist;
}
