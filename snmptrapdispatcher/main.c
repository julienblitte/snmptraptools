#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include "..\core\trapSnmp.h"
#include "..\core\pluginEngine.h"
#include "..\core\snmptraptools_config.h"
#include "configuration.h"

FILE *logFile;
static plugin_set plugins[MAX_PLUGINS];

void actionCallback(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber)
{
    plugin_set *target;

    fprintf(logFile, "  Rule matches for plugin #%08X: ", action->pluginUID);

	target = plugin_get_by_id(plugins, sizeof(plugins), action->pluginUID);
    if (target != NULL)
    {
    	fprintf(logFile, "%s\n", target->GetName());

		target->Run(trap);
    }
    else
    {
    	fprintf(logFile, "failed to identify plugin!\n");
    }
}

int main()
{
    char buffer[MAX_DISPATCHER_LINE_LEN];
    snmpTrap trap;
    trap_action_entry *trap_actions;
    bool recieving = false;
	char **plugin_dll;
	int i, j;

	char plugin_config[MAX_PLUGIN_CONFIG_LEN];
	uint32_t plugin_config_size;
	char *plugin_default_value;

    trap_actions = configurationLoad();
    if (trap_actions == NULL)
    {
        return EXIT_FAILURE;
    }

    logFile = fopen(DISTPATCHER_LOGFILE, "at");
    if (logFile == NULL)
    {
        return EXIT_FAILURE;
    }

	fprintf(logFile, "Dispatcher restart, reload plugins.\n");
	plugin_dll = plugin_find();
	i=0;
	j=0;
	while (plugin_dll[i] && j < MAX_PLUGINS)
	{
		fprintf(logFile, "  File %s: ", plugin_dll[i]);
		if (plugin_load(plugin_dll[i], &plugins[j]))
		{
			fprintf(logFile, "plugin %s loaded, id=%08X.\n", plugins[j].GetName(), plugins[j].GetUID());

			plugin_config_size = sizeof(plugin_config);
			if (!plugin_get_configuration(plugin_config, &plugin_config_size, plugins[j].GetName()))
			{
				// unable to get configuration, get default values
				plugin_default_value = plugins[j].EditConfig(NULL, &plugin_config_size);
				memcpy(plugin_config, plugin_default_value, plugin_config_size);
				plugin_set_configuration(plugin_config, plugin_config_size, plugins[j].GetName());
			}

			plugins[j].LoadConfig(plugin_config, plugin_config_size);

			j++;
		}
		else
		{
			fprintf(logFile, "load failure!\n");
		}
		i++;
	}
	fprintf(logFile, "\n");
	fflush(logFile);

    while(fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (recieving == false)
        {
            recieving = true;
            ZeroMemory(&trap, sizeof(snmpTrap));
            // variables_count is set to 0

            trap.date = time(NULL);
        }

        if (buffer[0] == '\n')
        {
            // reading complete
            recieving = false;
            snmptrap_print(logFile, &trap);

            configurationMatch(trap_actions, &trap, &actionCallback);

            fprintf(logFile, "\n");
            fflush(logFile);

            snmptrap_free(&trap);
        }
        else
        {
            snmptrap_gets(buffer, &trap);
        }
    }

    fclose(logFile);
    configurationClean(trap_actions);

    return EXIT_SUCCESS;
}
