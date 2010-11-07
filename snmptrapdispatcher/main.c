#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include "../libtrapsnmp/trapSnmp.h"
#include "../snmptraptools_config.h"
#include "configuration.h"
#include "xstring.h"

FILE *logFile;

const char *compileDescription(trap_action_entry *action, snmpTrap *trap)
{
    static char description[MAX_DESCRIPTION_LEN];
    char nameToken[16];
    int i;

    if (action->desc == NULL)
    {
        return "";
    }

    // copy description text
    strncpy(description, action->desc, sizeof(description));

    // handle ${*} item
    if (strstr(description, "${*}") != NULL)
    {
        for(i=0; i < trap->variablesCount; i++)
        {
            snprintf(nameToken, sizeof(nameToken), "${%d} ${*}", i+1);
            strnreplace(description, "${*}", nameToken, sizeof(description));
        }
        strnreplace(description, "${*}", "", sizeof(description));
    }

    // replace each values
    for(i=0; i < trap->variablesCount; i++)
    {
        snprintf(nameToken, sizeof(nameToken), "${%d}", i+1);
        strnreplace(description, nameToken, trap->variablesValue[i], sizeof(description));
    }

    return description;
}

void actionCallback(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber)
{
    static char param[MAX_PARAMETERS];

    fprintf(logFile, "run: [%s]\n", action->run);

    if (action->run != NULL)
    {
        snprintf(param, sizeof(param), "%s %s %ld %ld \"%s\"", trap->agent, trap->enterprise, trap->genericTrap, trap->specificTrap,
                 compileDescription(action, trap));
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

            fprintf(logFile, "\n");
            fflush(logFile);

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
