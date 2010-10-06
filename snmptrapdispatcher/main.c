#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include "../libtrapsnmp/trapSnmp.h"
#include "../snmptraptool_config.h"
#include "configuration.h"

FILE *logFile;

void actionCallback(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber)
{
    char param[MAX_NETWORK_ADDRESS_LEN+MAX_OID_LEN+64];

    fprintf(logFile, "run: [%s]\n", action->run);

    if (action->run != NULL)
    {
        snprintf(param, sizeof(param), "%s %s %ld %ld", trap->agent, trap->enterprise, trap->genericTrap, trap->specificTrap);
        ShellExecute(NULL, "open", action->run, param, action->wkDir, SW_SHOW);
    }
}

int main()
{
    char buffer[MAX_DISPATCHER_LINE_LEN];
    snmpTrap trap;
    trap_action_entry *trap_actions;
    bool recieving = false;


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

    while(fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (recieving == false)
        {
            recieving = true;
            ZeroMemory(&trap, sizeof(snmpTrap));
            // variablesCount is set to 0

            trap.date = time(NULL);
        }

        if (buffer[0] == '\n')
        {
            // reading complete
            recieving = false;
            trapPrint(logFile, &trap);

            configurationMatch(trap_actions, &trap, &actionCallback);

            trapFree(&trap);
        }
        else
        {
            trapReadLine(buffer, &trap);
        }
    }

    fclose(logFile);
    configurationClean(trap_actions);

    return EXIT_SUCCESS;
}
