#ifndef COMPILER_H
#define COMPILER_H

/*
    Macros to simple work with GCC

    Author: Michal Kukowski
    email: michalkukowski10@gmail.com

    LICENCE: GPL 3.0
*/

#include <stddef.h> /* offsetof */

/* use thic macro to convert to string  */
#define tostring(s)	#s

/* CONCAT use this macro to concat words in one string */
#define _concat(x, y) x ## y
#define concat(x, y) _concat(x, y)

/* set if () { } at the begining of pipeline */
#define likely(x) __builtin_expect(!!(x), 1)

/* set if () { } at the end of pipeline */
#define unlikely(x) __builtin_expect(!!(x), 0)

/* to avoid warning about uninit var */
#define init_var(x) ((x) = *(&(x)))

/* use this macro to get parent struct from member struct */
#define container_of(ptr, type, member) \
({ \
    const typeof(((type *)0)->member) *__mptr = (ptr);     \
    (type *) ((char *) __mptr - offsetof(type, member)); \
})

/* use this macro to assert types */
#define type_check(t1, t2) (!((t1 *)0 - (t2 *)0))

/* use this macro to assert types of variables */
#define type_var_check(v1, v2) type_check(typeof(v1), typeof(v2))

/* BUILTIN FUNC */

/* get index of least bit ( from left ) of -1 iff X == 0  */
#define least_1_int(x)          (__builtin_ffs(x) - 1)
#define least_1_long(x)         (__builtin_ffsl(x) - 1)
#define least_1_longlong(x)     (__builtin_ffsll(x) - 1)

/* get number of leading zeros */
#define leading_0_int(x)        (x ? __builtin_clz(x) : 0)
#define leading_0_long(x)       (x ? __builitn_clzl(x) : 0)
#define leading_0_longlong(x)   (x ? __builitn_clzll(x) : 0)

/* get number of trailing zeros */
#define trailing_0_int(x)       (x ? __builtin_ctz(x) : 0)
#define trailing_0_long(x)      (x ? __builtin_ctzl(x) : 0)
#define trailing_0_longlong(x)  (x ? __builtin_ctzll(x) : 0)

/* get number of ones */
#define number_1_int(x)         (x ? __builtin_popcount(x) : 0)
#define number_1_long(x)        (x ? __builtin_popcountl(x) : 0)
#define number_1_longlong(x)    (x ? __builtin_popcountll(x) : 0)

/* get parity number of 1 ( number of 1 % 2) */
#define parity_1_int(x)         (!__builtin_parity(x))
#define parity_1_long(x)        (!__builtin_parityl(x))
#define parit_1_longlong(x)     (!__builtin_parityll(x))

/* swap bytes in variable */
#define swap_bytes16(x)         (__builtin_bswap16(x))
#define swap_bytes32(x)         (__builtin_bswap32(x))
#define swap_bytes64(x)         (__builtin_bswap64(x))

/* alloc on stack by resize stack frame in init f call */
#define alloc_on_stack(size) __builtin_alloca(size)

/* alloc with align on stack by resize stack frame in f init call  */
#define alloc_on_stack_align(size, align) __builtin_alloca_with_align(size, align)

/* tell compiler that this code is unreachable */
#define unreachable  __builtin_unreachable

/* clear cache */
#define clear_cache(begin, end) __builtin___clear_cache((char *)begin, (char *)end)

#define WRITE_CACHE 1
#define READ_CACHE  0

#define CACHE_NO_SAVE           0
#define CACHE_SAVE_LOW_PRIO     1
#define CACHE_SAVE_NORMAL_PRIO  2
#define CACHE_SAVE_HIGH_PRIO    3

#define load_to_cache(addr, rw, prio)  __builtin_prefetch(addr, rw, prio)

/* FUNCTION ATTR */

/* use this macro instead of inline */
#define __inline__ inline __attribute__(( always_inline ))

/* function can't return, it exit immediately  */
#define __no_return__ __attribute__(( noreturn ))

/* f args can be null */
#define __nonull__(...) __attribute__(( nonnull(__VA_ARGS__) ))

/* f is used very few times, so put it in special part of text */
#define __unlikely_use__ __attribute__(( cold ))

/*f is used so many times so put it at the begining of text */
#define __likely_use__ __attribute__(( hot ))

#define PRIO_RESERVED 100

/* f is call before main ( init global data in this f ) prio >= 0 */
#define __before_main__(prio) __attribute__(( constructor(prio + PRIO_RESERVED + 1) ))

/* f is call after main or in exit() (dealloc global data in this f) prio >= 0 */
#define __after_main__(prio) __attribute__(( destructor(prio + PRIO_RESERVED + 1) ))

/* this function could be compiled in few version with smid support */
#define __simd__ __attribute__(( simd ))

/* tell compiler that this f or var could be nt used */
#define __unused__ __attribute__(( unused ))

/* tell compiler that function is weak and can be overwrite */
#define __weak__ __attribute__(( weak ))

/* VARIABLE ATTR */

/* align variable to bytes */
#define __align__(bytes) __attribute__(( aligned(bytes) ))

/* align to biggest aligment */
#define __max_align__(bytes) __attribute__((aligned (__BIGGEST_ALIGNMENT__) ))

/* packed variable / structure */
#define __packed__ __attribute__(( packed ))

/* garbage_collected by function with only 1 arg: pointer to this variable */
#define __garbage_collector__(func) __attribute__(( cleanup(func) ))

#endif
