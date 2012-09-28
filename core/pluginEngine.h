#ifndef H_PLUGINENGINE_INCLUDED
#define H_PLUGINENGINE_INCLUDED

#include <stdbool.h>
#include "plugin_common.h"
#include "trapSnmp.h"

#define TYPE(F)		_TYPE(F)
#define _TYPE(F)	_ ## F
#define NAME(F)		_NAME(F)
#define _NAME(F)	#F

typedef const char * (WINAPI * TYPE(GETNAME)) ();
typedef void (WINAPI * TYPE(LOADCONFIG)) (const void *data, const unsigned int data_size);
typedef void* (WINAPI * TYPE(EDITCONFIG)) (void *data, unsigned int *data_size);
typedef void (WINAPI * TYPE(RUN)) (snmpTrap*);
typedef uint32_t (WINAPI * TYPE(GETUID)) ();

typedef struct _plugin_set
{
	HMODULE dll;
	unsigned int UID;
	TYPE(GETNAME) GetName;
	TYPE(LOADCONFIG) LoadConfig;
	TYPE(EDITCONFIG) EditConfig;
	TYPE(RUN) Run;
	TYPE(GETUID) GetUID;
} plugin_set;

char **plugin_find();
bool plugin_load(const char *module, plugin_set *plugin);
bool plugin_isvalid(plugin_set *p);
void plugin_emit_sample(plugin_set *p);
plugin_set *plugin_get_by_id(plugin_set *array, unsigned int array_size, unsigned int UID);
bool plugin_set_configuration(void *data, uint32_t data_size, const char *plugin_name);
bool plugin_get_configuration(void *data, uint32_t *data_size, const char *plugin_name);

#endif // H_PLUGINENGINE_INCLUDED
