#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "pluginEngine.h"
#include "snmptraptools_config.h"


char **plugin_find()
{
	static char *files[MAX_PLUGINS];

	WIN32_FIND_DATA fd;
	HANDLE h;
	unsigned int n;

	memset(files, 0, sizeof(files));

	n = 0;
	h = FindFirstFile(PLUGIN_FILTER, &fd);
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

	plugin->GetName = (_GetName)GetProcAddress(plugin->dll, GETNAME);
	if (!plugin->GetName) goto plugin_load_error;

	plugin->LoadConfig = (_LoadConfig)GetProcAddress(plugin->dll, LOADCONFIG);
	if (!plugin->LoadConfig) goto plugin_load_error;

	plugin->EditConfig = (_EditConfig)GetProcAddress(plugin->dll, EDITCONFIG);
	if (!plugin->EditConfig) goto plugin_load_error;

	plugin->Run = (_Run)GetProcAddress(plugin->dll, RUN);
	if (!plugin->Run) goto plugin_load_error;

	plugin->GetUID = (_GetUID)GetProcAddress(plugin->dll, GETUID);
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

void plugin_get_configuration(void *data, uint32_t *data_size, const char *plugin_name)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PLUGIN_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        printf("Unable to access plugin key in registry!\n");
        return;
    }

	if (RegQueryValueEx(hKey, plugin_name, NULL, NULL, data, (LPDWORD)data_size) != ERROR_SUCCESS)
	{
		data_size = 0;
	}

	RegCloseKey(hKey);
	return;
}

bool plugin_set_configuration(void *data, uint32_t data_size, const char *plugin_name)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PLUGIN_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        printf("Unable to access plugin key in registry!\n");
        return false;
    }

	if (RegSetValueEx(hKey, plugin_name, 0, REG_BINARY, (BYTE*)data, data_size) != ERROR_SUCCESS)
    {
		printf("Unable to create value \"%s\" in plugin!\n", plugin_name);
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
		if (array->UID == UID)
		{
			return array;
		}
		array++;
		array_size--;
	}

	return NULL;
}

#ifdef DEBUG
int main()
{
	char **list = plugin_find();
	unsigned int i;
	plugin_set p[MAX_PLUGINS];
	char buffer[256];
	uint32_t buffer_size;

	i=0;
	while (list[i] && i < MAX_PLUGINS)
	{
		printf("%s: ", list[i]);

		if (plugin_load(list[i], &p[i]))
		{
			printf("[%p] %s (%u)\n", p[i].dll, p[i].GetName(), p[i].GetUID());
/*
			plugin_emit_sample(&p);  // Run test
*//*
			p.EditConfig(NULL, 0);  // EditConfig test
*//*
			buffer_size = sizeof(buffer);
			plugin_get_configuration(buffer, &buffer_size, p[i].GetName()); 	// get_configuration test
			buffer[buffer_size]='\0';
			printf("config:[%s]\n", buffer);
*//*
			strcpy(buffer, p.GetName());
			buffer_size = strlen(buffer);
			if (plugin_set_configuration(buffer, buffer_size, p.GetName()))		// set_configuration test
			{
				printf("Default settings created!\n");
			}
			else
			{
				printf("error!\n");
			}
*/
		}
		else
		{
			printf("unable to load plugin!\n");
		}
		i++;
	}

    return 0;
}
#endif // DEBUG
