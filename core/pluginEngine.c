#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "pluginEngine.h"
#include "snmptraptools_config.h"

BYTE *pluginDirectory(BYTE *path, DWORD pathSize)
{
	char *backslash;

	HKEY serviceKey;

	// default value
	strcpy((char*)path, PLUGIN_FILTER);

	// try to locate the plugin directory using service executable
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_SERVICE_PATH, 0, KEY_READ, &serviceKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(serviceKey, REGISTRY_SERVICE_EXECUTABLE, NULL, NULL, (BYTE*)path, &pathSize) == ERROR_SUCCESS)
		{
			backslash = strrchr((char*)path, '\\');
			if (backslash != NULL)
			{
				*backslash= '\0';
				strcat((char*)path, "\\");
				strcat((char*)path, PLUGIN_FILTER);
			}
		}
		RegCloseKey(serviceKey);
	}

	return (BYTE*)path;
}

char **plugin_find()
{
	static char *files[MAX_PLUGINS];

	char pluginPath[MAX_PATH];

	WIN32_FIND_DATA fd;
	HANDLE h;
	unsigned int n;

	pluginDirectory((BYTE*)pluginPath, sizeof(pluginPath));

	memset(files, 0, sizeof(files));

	n = 0;
	h = FindFirstFile(pluginPath, &fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		files[n++] = strdup(fd.cFileName);

		while (FindNextFile(h, &fd) && (n < 255))
		{
			files[n++] = strdup(fd.cFileName);
		}
	}

	return files;
}

bool plugin_load(const char *module, plugin_set *plugin)
{
	ZeroMemory(plugin, sizeof(plugin_set));

	plugin->dll = LoadLibrary(module);
	if (plugin == NULL)
	{
		return false;
	}

	plugin->GetName = (TYPE(GETNAME))GetProcAddress(plugin->dll, NAME(GETNAME));
	if (!plugin->GetName) goto plugin_load_error;

	plugin->LoadConfig = (TYPE(LOADCONFIG))GetProcAddress(plugin->dll, NAME(LOADCONFIG));
	if (!plugin->LoadConfig) goto plugin_load_error;

	plugin->EditConfig = (TYPE(EDITCONFIG))GetProcAddress(plugin->dll, NAME(EDITCONFIG));
	if (!plugin->EditConfig) goto plugin_load_error;

	plugin->Run = (TYPE(RUN))GetProcAddress(plugin->dll, NAME(RUN));
	if (!plugin->Run) goto plugin_load_error;

	plugin->GetUID = (TYPE(GETUID))GetProcAddress(plugin->dll, NAME(GETUID));
	if (!plugin->GetUID) goto plugin_load_error;

	plugin->UID = plugin->GetUID();

	return true;

plugin_load_error:
	plugin->dll = NULL;
	return false;
}

bool plugin_isvalid(plugin_set *p)
{
	return (p->dll != NULL && p->UID != 0);
}

void plugin_emit_sample(plugin_set *p)
{
	snmpTrap trap;

	if (!plugin_isvalid(p))
	{
		return;
	}

    trap.date = time(NULL);
    strncpy(trap.community, "public", MAX_COMMUNITY_LEN);
    trap.timestamp = 3600u;
    trap.generic_type = 6u;
    trap.specific_type = 100u;
    strncpy(trap.agent, "127.0.0.1", sizeof(trap.agent));
    strncpy(trap.enterprise, "1.3.6.1.4.1.22117", sizeof(trap.enterprise));
    trap.variables_count = 0;
    trap.variables_oid[0] = NULL;
    trap.variables_value[0] = NULL;

	p->Run(&trap);
}

bool plugin_get_configuration(void *data, uint32_t *data_size, const char *plugin_name)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PLUGIN_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to access plugin key in registry!\n");
#endif
        return false;
    }

	if (RegQueryValueEx(hKey, plugin_name, NULL, NULL, data, (LPDWORD)data_size) != ERROR_SUCCESS)
	{
		*data_size = 0;
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

bool plugin_set_configuration(void *data, uint32_t data_size, const char *plugin_name)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PLUGIN_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
#ifdef DEBUG
        printf("Unable to access plugin key in registry!\n");
#endif
        return false;
    }

	if (RegSetValueEx(hKey, plugin_name, 0, REG_BINARY, (BYTE*)data, data_size) != ERROR_SUCCESS)
    {
#ifdef DEBUG
		printf("Unable to create value \"%s\" in plugin!\n", plugin_name);
#endif
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

plugin_set *plugin_get_by_id(plugin_set *array, unsigned int array_size, unsigned int UID)
{
	while (array_size > 0)
	{
		if (array->UID == 0)
		{
			break;
		}
		if (array->UID == UID)
		{
			return array;
		}
		array++;
		array_size--;
	}

	return NULL;
}
