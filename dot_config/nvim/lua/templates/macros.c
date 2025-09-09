#include <stdio.h>
#include <string.h>
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#define SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#define CLEARBIT(V,I)    V[I>>3] &= ~(u8)(1<<(I&7))
#define TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0

#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

// permit static str
#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

// runtime conditional
#define DEBUG 1
#define INFO 2
int LEVEL=DEBUG;

#define debug(...) \
            do { if (LEVEL>=DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#define info(...) \
            do { if (LEVEL>=INFO) fprintf(stderr, __VA_ARGS__); } while (0)



// more involved - timestamps, file, line
#include <time.h>
#define debug(...) \
    do { \
        if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG) { \
            time_t now = time(NULL); \
            struct tm *tm_info = localtime(&now); \
            char timestamp[20]; \
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
            fprintf(stderr, "[DEBUG][%s][%s:%d] ", timestamp, __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
        } \
    } while (0)

#define info(...) \
    do { \
        if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO) { \
            time_t now = time(NULL); \
            struct tm *tm_info = localtime(&now); \
            char timestamp[20]; \
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
            fprintf(stderr, "[INFO][%s][%s:%d] ", timestamp, __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
        } \
    } while (0)





/* not most efficient, but most pleasant */
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// improved filena
#define FILENAME \
    ({ \
        const char *slash = strrchr(__FILE__, '/'); \
        slash ? slash + 1 : __FILE__; \
    })


// or via func
static inline const char *extract_filename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}
#define FILENAME (extract_filename(__FILE__))


// ==== START: Current goto ===
#define FILENAME \
    ({ \
        const char *slash = strrchr(__FILE__, '/'); \
        slash ? slash + 1 : __FILE__; \
    })

#define _log_with_level(level, ...) \
    do { \
        time_t now = time(NULL); \
        struct tm *tm_info = localtime(&now); \
        char timestamp[20]; \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info); \
	fprintf(stderr, "[%s][%s][%s:%d] " fmt "\n",                     \
		timestamp, level, FILENAME, __LINE__, ##__VA_ARGS__);    \
    } while (0)

#define debug(...) \
    do { if (LEVEL <= DEBUG) _log_with_level("DEBUG", ##__VA_ARGS__); } while (0)

#define info(...) \
    do { if(LEVEL <= INFO) _log_with_level("INFO", ##__VA_ARGS__); } while (0)

// ==== END: Current goto ===
//
// disablint comptime

#ifndef CURRENT_DEBUG_LEVEL
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

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




// gcc only

#define DBG(expr) ({int g2rE3=expr; fprintf(stderr, "%s:%d:%s(): ""%s->%i\n", __FILE__,  __LINE__, __func__, #expr, g2rE3); g2rE3;})
//usage
//before
fprintf(stderr, "%i\n", (1*2*3*4*5*6));
// 720
//after
fprintf(stderr, "%i\n", DBG(1*2*3*4*5*6));
// hello.c:86:main(): 1*2*3*4*5*6->720
// 720
// nesting
/*

DBGI(printf("%i\n", DBGI(1*2*3*4*5*6)));
hello.c:86:main(): 1*2*3*4*5*6->720
// 720
hello.c:86:main(): printf("%i\n", DBGI(1*2*3*4*5*6))->4
 */
