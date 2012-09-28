#ifndef PLUGIN_COMMON_H_INCLUDED
#define PLUGIN_COMMON_H_INCLUDED

#include <stdint.h>
#include "trapSnmp.h"

#define GETNAME		GetName
#define LOADCONFIG	LoadConfig
#define EDITCONFIG	EditConfig
#define RUN			Run
#define GETUID		GetUID

#ifdef BUILD_DLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

DLL_EXPORT const char *GETNAME();
DLL_EXPORT void LOADCONFIG(const void *data, const unsigned int data_size);
DLL_EXPORT void* EDITCONFIG(void *data, unsigned int *data_size);
DLL_EXPORT void RUN(snmpTrap *trap);
DLL_EXPORT uint32_t GETUID();

#define STR2UID(A,B,C,D)	((((uint32_t)D)<<24)|(((uint32_t)C)<<16)|(((uint32_t)B)<<8)|((uint32_t)A))

#endif // PLUGIN_COMMON_H_INCLUDED
