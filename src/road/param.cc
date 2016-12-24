/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2007
---------------------------------------------------------------------------- */

#include "param.h"
#include <string>
#include <string.h>

const char* Param::read(const int a_argc, const char** a_argv,
                        const char* a_param,
                        const char* a_def)
{
    for (int i=0; i<a_argc; i++)
        if (strcmp(a_argv[i], a_param) == 0)
        {
            if (i+1 == a_argc)
                return "";
            else
                return a_argv[i+1];
        }
    return a_def;
}
