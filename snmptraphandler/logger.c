#include <stdio.h>
#include <windows.h>
#include "logger.h"
#include "config.h"

// temporary implementation to be changed with a newer one

#define LOG_BLANK       (LOG_PEDANTIC-100)

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
    return logPrint(LOG_BLANK, "-");
}

BOOL logPrint(INT32 level, char *data)
{
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
        fprintf(logFile, "%s\t%s\n", gLogLevelName[level], data);
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

void logPrintIntVar(INT32 level, char *variableName, DWORD value)
{
        char data[256];

        snprintf(data, sizeof(data), "%s: %lu", variableName, value);
        logPrint(level, data);
}



void logPrintPtrVar(char *variableName, void *address)
{
        char data[256];

        snprintf(data, sizeof(data), ">>> @%s = %p", variableName, address);
        logPrint(LOG_PEDANTIC, data);
}
