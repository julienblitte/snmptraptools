#ifndef __PLUGIN_SYSLOG_H__
#define __PLUGIN_SYSLOG_H__

#include <windows.h>
#include "..\core\trapSnmp.h"

#define PLUGIN_NAME		"syslog forward"
// 'SYSL'
#define PLUGIN_UID		((unsigned int)'LSYS')

#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

DLL_EXPORT const char *GetName();
DLL_EXPORT void LoadConfig(void *data, unsigned int data_size);
DLL_EXPORT void* EditConfig(void *data, unsigned int *data_size);
DLL_EXPORT void Run(snmpTrap *trap);
DLL_EXPORT unsigned int GetUID();

#include <stdlib.h>
#include <stdint.h>

typedef struct _plugin_configuration
{
	char target[16];
	char application[32];
} plugin_configuration;

#define SYSLOG_FACILITY_USER_LEVEL	1
#define SYSLOG_SEVERITY_NOTICE		5

#define SYSLOG_SERVER_PORT	514

#endif // __PLUGIN_SYSLOG_H__

