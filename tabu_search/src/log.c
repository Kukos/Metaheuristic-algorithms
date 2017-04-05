#include <log.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h> 	/* stat */
#include <time.h> 		/* strftime */
#include <string.h> 	/* strlen, SSE  */
#include <fcntl.h>
#include <compiler.h>

static __unused__ FILE *logfd = NULL;
static __unused__ FILE *logfile = NULL;
static __unused__ const char const *logdir = "./logs";

#define LOGBUF_SIZE	128

#ifdef DEBUG_MODE
void __log__(const char *msg, ...)
{
	va_list args;

    va_start(args, msg);
	(void)vfprintf(logfd == NULL ? stderr : logfd, msg, args);
	va_end(args);

	if (logfile != NULL)
	{
		va_start (args, msg);
		(void)vfprintf(logfile, msg, args);
		va_end(args);
	}
}
#else
void __log__(const char *msg, ...) { }
#endif

#ifdef TRACE_MODE
void __trace_call__(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
	(void)vfprintf(logfd == NULL ? stderr : logfd, msg, args);
	va_end(args);

	if (logfile != NULL)
	{
		va_start (args, msg);
		(void)vfprintf(logfile, msg, args);
		va_end(args);
	}
}
#else
void __trace_call__(const char *msg, ...) { }
#endif

#ifndef SILENT_ERROR
void __error__(const char *msg, ...)
{
	va_list args;

    va_start(args, msg);
	(void)vfprintf(logfd == NULL ? stderr : logfd, msg, args);
	va_end(args);

	if (logfile != NULL)
	{
		va_start (args, msg);
		(void)vfprintf(logfile, msg, args);
		va_end(args);
	}
}
#else
void __error__(const char *msg, ...) { }
#endif

#ifdef DEBUG_MODE
int log_init(FILE *fd, int to_file)
{
	struct stat st;
	struct tm t;
	time_t now;

	char buf[LOGBUF_SIZE];
	size_t len;

	if (fd == NULL || fd != stderr || fd != stdout)
		logfd = stderr;
	else
		logfd = fd;

	if (to_file == LOG_TO_FILE)
	{
		/* create directory if doesn't exist */
		if (stat(logdir, &st) == -1)
			if (mkdir(logdir, 0775) == -1)
			{
				perror("mkdir error\n");
				return 1;
			}

		len = strlen(logdir);
		if (memcpy((void*)buf, (void*)logdir, len) == NULL)
		{
			perror("memcpy error\n");
			return 1;
		}

		/* get local time and create string - file name */
		now = time(NULL);
		t = *localtime(&now);
		if (strftime(buf + len, LOGBUF_SIZE, "/%Y%m%d%H%M%S.log", &t) == 0)
		{
			perror("strftime error\n");
			return 1;
		}

		/* check existing of file */
		if (stat(buf, &st) == -1)
		{
			if (creat(buf, 0644) == -1)
			{
				perror("creat error\n");
				return 1;
			}
		}
		else
		{
		 	perror("Logs file exists\n");
			return 1;
		}

		/* open */
		logfile = fopen(buf, "w");
		if (logfile == NULL)
		{
			perror("fopen error\n");
			return 1;
		}
	}
	else
		logfile = NULL;

	return 0;
}
#else
int log_init(FILE *fd, int to_file) { return 0; }
#endif

#ifdef DEBUG_MODE
void log_deinit(void)
{
	if (logfile != NULL)
	{
		(void)fclose(logfile);
		logfile = NULL;
	}

	logfd = NULL;
}
#else
void log_deinit(void) { }
#endif
