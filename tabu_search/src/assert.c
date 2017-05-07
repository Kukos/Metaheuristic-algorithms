#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

void __assert_msg(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    (void)vfprintf(stderr, msg, args);
    va_end(args);
}
