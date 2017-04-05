#ifndef ASSERT_H
#define ASSERT_H

#include <compiler.h>   /* tostring */
#include <stdarg.h>     /* va_args */
#include <stdio.h>      /* vfprintf */
#include <stdlib.h>     /* exit */

#define ASSERT_MSG \
    "[ASSERT]\tFILE: %s\n\tFUNC: %s\tLINE: %d\n", __FILE__, __func__, __LINE__

void __assert_msg(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    (void)vfprintf(stderr, msg, args);
    va_end(args);
}

#ifdef ASSERT
    #define assert(expr) \
    do { \
        if (expr) \
        { \
            __assert_msg(ASSERT_MSG); \
            __assert_msg("FAILED:\t" tostring(expr) "\n", ""); \
            exit(1); \
        } \
    } while (0);
#else
    #define assert(expr)
#endif

#endif
