#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#include "trapSnmp.h"
#include "../snmptraptools_config.h"

void trapFree(snmpTrap *trap)
{
    while(trap->variablesCount > 0)
    {
        free(trap->variablesOID[trap->variablesCount]);
        free(trap->variablesValue[trap->variablesCount]);

        trap->variablesCount--;
    }
}

void trapPrint(FILE *out, snmpTrap *trap)
{
    struct tm *ts;
    char date[128];
    unsigned int i;

    ts = localtime(&trap->date);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", ts);

    fprintf(out, "%s trap (%ld, %ld) recieved from %s\n",
            date, trap->genericTrap, trap->specificTrap, trap->agent);

    fprintf(out, "  Community: %s\n", trap->community);
    fprintf(out, "  Uptime: %lu\n", trap->timestamp);
    fprintf(out, "  Generic type: %ld\n", trap->genericTrap);
    fprintf(out, "  Specific type: %ld\n", trap->specificTrap);
    fprintf(out, "  Address of agent: %s\n", trap->agent);
    fprintf(out, "  Enterprise OID: %s\n", trap->enterprise);
    fprintf(out, "  Given values (%lu): \n", trap->variablesCount);

    for(i=0; i < trap->variablesCount; i++)
    {
        fprintf(out, "    %s: %s\n", trap->variablesOID[i], trap->variablesValue[i]);
    }

    fflush(out);
}

long oidLastId(const char *oid)
{
    char *id;

    id = strrchr(oid, OID_SEPARATOR);
    if (id == NULL)
    {
        return 0;
    }

    if (id[0] == '\0')
    {
        return 0;
    }

    id++;

    return atol(id);
}

BOOL oidStartBy(const char *begin, const char *str)
{
    int i;

    i=0;
    while(begin[i] != '\0')
    {
        // no end of string control is necessary for parameter 'str'
        // because if str[i] == '\0', the next condition will be true
        if (begin[i] != str[i])
        {
            return FALSE;
        }
        i++;
    }

    return TRUE;
}

BOOL oidIsValid(const char *oid)
{
    BOOL dot;

    dot = TRUE; // first character must be a digit
    while(*oid != '\0')
    {
        if (*oid == OID_SEPARATOR)
        {
            if (dot) // double dot
            {
                return FALSE;
            }
            dot = TRUE;
        }
        else if (*oid >= '0' && *oid <= '9' )
        {
            dot = FALSE;
        }
        else
        {
            return FALSE;
        }
        oid++;
    }

    if (dot == TRUE) // one loop is mandatory and last character must not be a dot
    {
        return FALSE;
    }

    return TRUE;
}

void trapReadLine(char *buffer, snmpTrap *trap)
{
        char *oid, *value;

        oid = strtok(buffer, "\t\n");
        value = strtok(NULL, "\t\n");

        if (oidIsValid(oid) != TRUE)
        {
            // not-valid oid
            return;
        }

        if (oidStartBy(OID_COMMUNITY, oid))
        {
            strncpy(trap->community, value, sizeof(trap->community));
        }
        else if (oidStartBy(OID_TIMESTAMP, oid))
        {
            sscanf(value, "%lu", &trap->timestamp);
        }
        else if (oidStartBy(OID_AGENT, oid))
        {
            strncpy(trap->agent, value, sizeof(trap->agent));
        }
        else if (oidStartBy(OID_ENTERPRISE, oid))
        {
            strncpy(trap->enterprise, value, sizeof(trap->enterprise));
        }
        else if (oidStartBy(OID_TRAP_CODE, oid))
        {
            if (oidStartBy(OID_TRAP_CODE_GENERIC, value))
            {
                // warning: 1 is added to last OID for generic trap to avoid '0' value
                trap->genericTrap = oidLastId(value) - 1;
                trap->specificTrap = 0;
            }
            else
            {
                trap->genericTrap = SPECIFIC_TYPE_GENERIC;
                trap->specificTrap = oidLastId(value);
            }
        }
        else
        {
            trap->variablesOID[trap->variablesCount] = malloc(strlen(oid)+1);
            strncpy(trap->variablesOID[trap->variablesCount], oid, (strlen(oid)+1));

            trap->variablesValue[trap->variablesCount] = malloc(strlen(value)+1);
            strncpy(trap->variablesValue[trap->variablesCount], value, (strlen(value)+1));

            trap->variablesCount++;
        }
}
