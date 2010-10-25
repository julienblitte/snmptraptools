#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include "logger.h"
#include "../snmptraptools_config.h"

// temporary implementation to be changed with a newer one

const char *gLogLevelName[] = { "PEDANTIC", "DEBUG", "INFO", "WARN", "ERROR" };

INT32 gLogLevel = LOG_INFORMATION;
FILE *logFile = NULL;

INT32 logGetLevel()
{
    return gLogLevel;
}

void logSetLevel(INT32 logLevel)
{
    gLogLevel = logLevel;
}

BOOL logInit()
{
    // force to open logfile
    return logPrintf(LOG_PEDANTIC-1, "");
}

BOOL logPrintf(INT32 level, char *format, ...)
{
    va_list va;

    va_start(va, format);

    if (logFile == NULL)
    {
        logFile = fopen(LOG_FILENAME, "wt");
        if (logFile == NULL)
        {
            return FALSE;
        }
    }

    if (level >= gLogLevel)
    {
        fprintf(logFile, "%s\t", gLogLevelName[level]);
        vfprintf(logFile, format, va);
        fflush(logFile);
    }

    return TRUE;
}

void logClean()
{
    if (logFile != NULL)
    {
        fclose(logFile);
        logFile = NULL;
    }
}
