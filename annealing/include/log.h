#ifndef LOG_H
#define LOG_H

/*
	Wrapper to simple log and errors

	Author: Michal Kukowski
	email: michalkukowski10@gmail.com

	LICENCE: GPL 3.0
*/

#include <stdio.h>

/*
	Modes to debug:
	DEBUG_MODE -> if defined, log works

	TRACE_MODE -> if defined trace_call works

    SILENT_ERROR -> if defined error logged errors

    Usage:
    1. First init logs: log_init
        IF you want to store logs in file pass LOG_TO_FILE to function
    2. Normal usage: TRACE, LOG, ERROR in your functions
    3. Clean log system: log_deinit

*/

#define __TRACE__   "[TRACE]\tFUNC: %s\n", __func__
#define __ERROR__   "[ERROR]\tFILE: %s\n\tFUNC: %s\tLINE: %d\n", __FILE__, __func__, __LINE__
#define __LOG__     "[LOG]\tFUNC: %s\tLINE: %d\n", __func__, __LINE__

#define TRACE(...) \
    do { \
        __trace_call__(__TRACE__, ##__VA_ARGS__); \
    } while (0)

#define LOG(msg, ...) \
    do { \
        __log__(__LOG__); \
        __log__("\t" msg, ##__VA_ARGS__); \
    } while (0)

#define ERROR(msg, errno, ...) \
    do { \
        __error__(__ERROR__); \
        __error__("\t" msg, ##__VA_ARGS__); \
        \
        return errno; \
    } while (0)

#define NO_LOG_TO_FILE  0
#define LOG_TO_FILE     1

/*
	Simple log function
	FUNCTION WORKS IFF DEBUG_MODE is defined

	PARAMS
	@IN msg - log message

	RETURN:
	This is a void function
*/
void __log__(const char *msg, ...);

/*
	Simple trace calling funcion

	FUNCTION WORKS IFF TRACE_MODE is defined

	PARAMS
	@IN msg - trace message

	RETURN:
	This is a void function
*/
void __trace_call__(const char *msg, ...);

/*
	Simple log error function
	FUNCTION WORKS IFF SILENT_ERROR is not defined

	PARAMS
	@IN msg - log message

	RETURN:
	This is a void function
*/
void __error__(const char *msg, ...);

/*
    initialize log system

    PARAMS
    @IN fd - FILE*, if NULL use default stderr
    @IN to_file log to special file ? LOG_TO_FILE - yes, NO_LOG_TO_FILE no

    RETURN
    0 iff success
    non-zero value iff failure
*/
int log_init(FILE *fd, int to_file);

/*
    deinit log system

    PARAMS
    NO PARAMS

    RETURN
    This is a void function
*/
void log_deinit(void);

#endif
