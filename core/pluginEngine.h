#ifndef H_PLUGINENGINE_INCLUDED
#define H_PLUGINENGINE_INCLUDED

#include "trapSnmp.h"
#include <stdbool.h>

#define GETNAME		"GetName"
#define LOADCONFIG	"LoadConfig"
#define EDITCONFIG	"EditConfig"
#define RUN			"Run"
#define GETUID		"GetUID"

typedef const char * (WINAPI * _GetName) ();
typedef void (WINAPI * _LoadConfig) (void *data, unsigned int data_size);
typedef void* (WINAPI * _EditConfig) (void *data, unsigned int *data_size);
typedef void (WINAPI * _Run) (snmpTrap*);
typedef unsigned int (WINAPI * _GetUID) ();

typedef struct _plugin_set
{
	HMODULE dll;
	unsigned int UID;
	_GetName GetName;
	_LoadConfig LoadConfig;
	_EditConfig EditConfig;
	_Run Run;
	_GetUID GetUID;
} plugin_set;

char **plugin_find();
bool plugin_load(const char *module, plugin_set *plugin);
bool plugin_isvalid(plugin_set *p);
void plugin_emit_sample(plugin_set *p);

#endif // H_PLUGINENGINE_INCLUDED
