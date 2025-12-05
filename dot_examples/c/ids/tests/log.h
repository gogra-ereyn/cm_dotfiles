#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_LEVEL_ERROR 0
#define DEBUG_LEVEL_INFO 1
#define DEBUG_LEVEL_DEBUG 2

#ifndef CURRENT_DEBUG_LEVEL
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

static inline const char *extract_filename(const char *path)
{
	const char *slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

#define FILENAME (extract_filename(__FILE__))

#define _log_with_level(level, fmt, ...)                                      \
	do {                                                                 \
		time_t now = time(NULL);                                     \
		struct tm tm_info;                                           \
		localtime_r(&now, &tm_info);                                 \
		char timestamp[20];                                          \
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S",  \
			 &tm_info);                                          \
		fprintf(stderr, "[%s][%s][%s:%s:%d] " fmt "\n", timestamp,   \
			level, __func__, FILENAME, __LINE__, ##__VA_ARGS__); \
	} while (0)

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define debug(fmt, ...) _log_with_level("DEBUG", fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void)0)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define info(fmt, ...) _log_with_level("INFO", fmt, ##__VA_ARGS__)
#else
#define info(fmt, ...) ((void)0)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define error(fmt, ...) _log_with_level("ERROR", fmt, ##__VA_ARGS__)
#else
#define error(fmt, ...) ((void)0)
#endif

#define t_error(status, fmt, ...)          \
	do {                               \
		error(fmt, ##__VA_ARGS__); \
		exit(status);              \
	} while (0)

#endif // LOG_H
