#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include "logger.h"
#include "../snmptraptool_config.h"

// temporary implementation to be changed with a newer one

const char *gLogLevelName[] = { "PEDANTIC", "DEBUG", "INFO", "WARN", "ERROR" };

INT32 gLogLevel = 0;
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
    return logPrintf(LOG_PEDANTIC-1, "");
}

BOOL logPrintf(INT32 level, char *format, ...)
{
    va_list va;

    if (logFile == NULL)
    {
        logFile = fopen(LOG_FILENAME, "wt");
        if (logFile == NULL)
        {
            return FALSE;
        }
    }

    if (level >= LOG_DEBUG)
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
