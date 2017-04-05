#ifndef COMMON_H
#define COMMON_H

/*
    Set of common useful functions

    Author: Michal Kukowski
    email: michalkukowski10@gmail.com

    LICENCE: GPL 3.0
*/

#include <compiler.h>

/* bits operations */
#define BIT(n)          (1ull << (n))
#define GET_BIT(n ,k)   (((n) & BIT(k) >> k)
#define SET_BIT(n, k)   ((n) |= BIT(k))
#define CLEAR_BIT(n, k) ((n) &= BIT(k))

/* swap a with b IFF types are the same */
#define SWAP(a ,b) \
    do{ \
        (void)type_var_check(a, b); \
        typeof(a) __temp = a; \
        a = b; \
        b = __temp; \
    } while (0);

/* free & NULL */
#define FREE(PTR) \
    do { \
        free(PTR); \
        PTR = NULL; \
    } while (0);

/* get array size IFF is declared on stack */
#define ARRAY_SIZE(ARR) (sizeof(ARR) / sizeof(ARR[0]))

/* get min IFF types are the same */
#define MIN(a, b) \
    __extension__ \
     ({ \
        (void)type_var_check(a ,b); \
        typeof(a) __a = (a); \
        typeof(b) __b = (b); \
        __a < __b ? __a : __b; \
    })

/* get max IFF types are the same */
#define MAX(a, b) \
    __extension__ \
    ({ \
        (void)type_var_check(a ,b); \
        typeof(a) __a = (a); \
        typeof(b) __b = (b); \
        __a > __b ? __a : __b; \
    })

#define ABS(x) \
    __extension__ \
    ({ \
        typeof(x) __x = (x); \
        __x < 0 ? -__x : __x; \
    })

#define ODD(x)          ((x) & 1)
#define EVEN(x)         (!((x) & 1))
#define CAST_TO_BOOL(x) (!!(x))

/* hamming weight */
#define HAMM_WEIGHT_int(n)          (number_1_int(n))
#define HAMM_WEIGHT_long(n)         (number_1_long(n))
#define HAMM_WEIGHT_longlong(n)     (number_1_longlong(n))

/* hamming distance */
#define HAMM_DIST_int(n, k)        (number_1_int((n) ^ (k)))
#define HAMM_DIST_long(n, k)       (number_1_long((n) ^ (k)))
#define HAMM_DIST_longlong(n, k)   (number_1_longlong((n) ^ (k)))

/* rly fast way to count log2 of n */
#define LOG2_int(n)             ((sizeof(typeof(n)) << 3) - leading_0_int(n))
#define LOG2_long(n)            ((sizeof(typeof(n)) << 3) - leading_0_long(n))
#define LOG2_longlong(n)        ((sizeof(typeof(n)) << 3) - leading_0_longlong(n))

/* ESCAPE COLORS */
#define RESET           "\033[0m"
#define BLACK           "\033[30m"
#define RED             "\033[31m"
#define GREEN           "\033[32m"
#define YELLOW          "\033[33m"
#define BLUE            "\033[34m"
#define MAGENTA         "\033[35m"
#define CYAN            "\033[36m"
#define WHITE           "\033[37m"
#define BOLDBLACK       "\033[1m\033[30m"
#define BOLDRED         "\033[1m\033[31m"
#define BOLDGREEN       "\033[1m\033[32m"
#define BOLDYELLOW      "\033[1m\033[33m"
#define BOLDBLUE        "\033[1m\033[34m"
#define BOLDMAGENTA     "\033[1m\033[35m"
#define BOLDCYAN        "\033[1m\033[36m"
#define BOLDWHITE       "\033[1m\033[37m"

#define printf_color(color, fmt, ...) \
    print(color fmt RESET, ##__VA_ARGS__);

#define fprintf_color(color, fd, fmt, ...) \
    fprintf(fd, color fmt RESET, ##__VA_ARGS__);

#endif
