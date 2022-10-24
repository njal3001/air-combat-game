#include "log.h"
#include <stdarg.h>
#include <stdio.h>

#define BUFSIZE 1024
char log_buf[BUFSIZE];

void log_info(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vsnprintf(log_buf, BUFSIZE - 1, msg, args);

    printf("%s\n", log_buf);
}

void log_warn(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vsnprintf(log_buf, BUFSIZE - 1, msg, args);

    printf("WARNING: %s\n", log_buf);
}

void log_err(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vsnprintf(log_buf, BUFSIZE - 1, msg, args);

    printf("ERROR: %s\n", log_buf);
}
