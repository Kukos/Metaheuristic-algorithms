#ifndef ASSERT_H
#define ASSERT_H

#include <compiler.h>   /* tostring */
#include <stdlib.h>     /* exit */

void __assert_msg(const char *msg, ...);

#define ASSERT_MSG \
    "[ASSERT]\tFILE: %s\n\tFUNC: %s\tLINE: %d\n", __FILE__, __func__, __LINE__

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
