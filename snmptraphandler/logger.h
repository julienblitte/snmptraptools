#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include "../snmptraptool_config.h"

INT32 logGetLevel();
void logSetLevel(INT32 logLevel);
BOOL logInit();
BOOL logPrintf(INT32 level, char *format, ...);
void logClean();
