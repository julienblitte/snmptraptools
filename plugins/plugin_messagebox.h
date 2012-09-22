#ifndef __PLUGIN_MESSAGEBOX_H__
#define __PLUGIN_MESSAGEBOX_H__

#include <windows.h>
#include <stdbool.h>

#include "..\core\trapSnmp.h"

#define PLUGIN_NAME		"messagebox"
// 'MSGB'
#define PLUGIN_UID		((unsigned int)'BGSM')

#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

DLL_EXPORT const char *GetName();
DLL_EXPORT void LoadConfig(const void *data, const unsigned int data_size);
DLL_EXPORT void* EditConfig(void *data, unsigned int *data_size);
DLL_EXPORT void Run(snmpTrap *trap);
DLL_EXPORT unsigned int GetUID();

#endif // __PLUGIN_MESSAGEBOX_H__
