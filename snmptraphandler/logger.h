#include <stdio.h>
#include <windows.h>

// temporary implementation to be changed with a newer one

#define LOG_ERROR       4
#define LOG_WARNING     3
#define LOG_INFORMATION 2
#define LOG_DEBUG       1
#define LOG_PEDANTIC    0

INT32 logGetLevel();
void logSetLevel(INT32 logLevel);
BOOL logInit();
BOOL logPrint(INT32 level, char *data);
void logClean();
void logPrintIntVar(INT32 level, char *variableName, DWORD value);
void logPrintPtrVar(char *variableName, void *address);
