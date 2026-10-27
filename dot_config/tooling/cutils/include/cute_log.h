#ifndef CUTE_LOG_H
#define CUTE_LOG_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#define CUTE_LOG_ERROR 0
#define CUTE_LOG_INFO  1
#define CUTE_LOG_DEBUG 2

#ifndef CUTE_LOG_LEVEL
#define CUTE_LOG_LEVEL CUTE_LOG_INFO
#endif

static inline const char *cute_extract_filename(const char *path)
{
	const char *slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

#define _CUTE_FILENAME (cute_extract_filename(__FILE__))

#define _cute_log(level, fmt, ...)                                             \
	do {                                                                   \
		time_t now = time(NULL);                                       \
		struct tm tm_info;                                             \
		localtime_r(&now, &tm_info);                                   \
		char timestamp[20];                                            \
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S",   \
			 &tm_info);                                            \
		fprintf(stderr, "[%s][%s][%s:%s:%d] " fmt "\n", timestamp,     \
			level, __func__, _CUTE_FILENAME, __LINE__,             \
			##__VA_ARGS__);                                        \
	} while (0)

#if CUTE_LOG_LEVEL >= CUTE_LOG_DEBUG
#define cute_debug(fmt, ...) _cute_log("DEBUG", fmt, ##__VA_ARGS__)
#else
#define cute_debug(fmt, ...) ((void)0)
#endif

#if CUTE_LOG_LEVEL >= CUTE_LOG_INFO
#define cute_info(fmt, ...) _cute_log("INFO", fmt, ##__VA_ARGS__)
#else
#define cute_info(fmt, ...) ((void)0)
#endif

#if CUTE_LOG_LEVEL >= CUTE_LOG_ERROR
#define cute_error(fmt, ...) _cute_log("ERROR", fmt, ##__VA_ARGS__)
#else
#define cute_error(fmt, ...) ((void)0)
#endif

static inline void cute_die(int status, int errnum, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	if (errnum)
		fprintf(stderr, ": %s", strerror(errnum));
	fprintf(stderr, "\n");
	va_end(args);
	exit(status);
}

#endif /* CUTE_LOG_H */
