/**
 * @file UcrtIobFunc.c
 * @brief __iob_func for MinGW toolchains linked with the Universal CRT
 * @author Seb
 * @date 2026-11-12
**/

#include <stdio.h>

static FILE *iobFunc(void)
{
    return __acrt_iob_func(0);
}

/* libFLAC was compiled with the dllimport declaration of __iob_func: it calls through __imp___iob_func */
FILE *(*__MINGW_IMP_SYMBOL(__iob_func))(void) = iobFunc;
