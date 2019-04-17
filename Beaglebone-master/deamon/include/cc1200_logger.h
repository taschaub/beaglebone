#ifndef CC1200_LOGGER
#define CC1200_LOGGER

/*#include "plccd.h"*/
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#define LOG_FILE  "/var/log/plccd.log"

extern FILE* logfile;
extern time_t ltime;
//#ifdef DEBUG
//Log macros in case of enabled debug
/* #define log_message(format, args...) do{		    \ */
/*     log_open();     				            \ */
/*     fprintf(logfile,"%s" format,timestamp() ##__VA_ARGS__); \ */
/*     log_close();				            \ */
/*     }while(0) */
#define log_int_message(integer) log_message("%i", integer)
//#else
//Empty macros in case debug is disabled
//#define log_message(fmt, ...) {}
//#define log_int_message(integer) {}
//x#endif

/**
 * initialize logging for daemon
 * @param filename the file to write the log to
 **/
void log_init(const char* filename);


void log_message(char *, ...);
/**
 * close logging
 **/
void log_close(void);

#endif
