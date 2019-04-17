#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cc1200_logger.h"
#include <time.h>

FILE* logfile=NULL;
time_t ltime;

void log_init(const char* filename)
{
    if( !(logfile=fopen(filename, "a+"))){
        fprintf(stderr, "Could not open file\n");
    }
}


void log_close()
{
    if(logfile) fclose(logfile);
    logfile = NULL;
}

void log_message(char * str, ...)
{
    if(logfile != NULL){
            char buffer[1024];
	    char buf_time[256];
	    time_t r_time;
	    struct tm *time_nfo;
	    va_list ap;

	    time(&r_time);
	    time_nfo = localtime(&r_time);
	    strftime(buf_time, sizeof(buf_time), "%d-%m - %X", time_nfo); 
            /* safely prefix the format string with [thread_id: %x] */
            va_start(ap, str);
	    vsnprintf(buffer,sizeof(buffer), str, ap);
            va_end(ap);
	    fprintf(logfile, "[%s] %s", buf_time, buffer);
            fflush(logfile);
    }
}
