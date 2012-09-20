#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#include "trapSnmp.h"
#include "..\core\snmptraptools_config.h"

void snmptrap_free(snmpTrap *trap)
{
    while(trap->variables_count > 0)
    {
        free(trap->variables_oid[trap->variables_count]);
        free(trap->variables_value[trap->variables_count]);

        trap->variables_count--;
    }
}

void snmptrap_print(FILE *out, snmpTrap *trap)
{
    struct tm *ts;
    char date[128];
    unsigned int i;

    ts = localtime(&trap->date);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", ts);

    fprintf(out, "%s trap (%u, %u) recieved from %s\n",
            date, trap->generic_type, trap->specific_type, trap->agent);

    fprintf(out, "  Community: %s\n", trap->community);
    fprintf(out, "  Uptime: %u\n", trap->timestamp);
    fprintf(out, "  Generic type: %u\n", trap->generic_type);
    fprintf(out, "  Specific type: %u\n", trap->specific_type);
    fprintf(out, "  Address of agent: %s\n", trap->agent);
    fprintf(out, "  Enterprise OID: %s\n", trap->enterprise);
    fprintf(out, "  Given values (%u): \n", trap->variables_count);

    for(i=0; i < trap->variables_count; i++)
    {
        fprintf(out, "    %s: %s\n", trap->variables_oid[i], trap->variables_value[i]);
    }

    fflush(out);
}

long snmpoid_last_id(const char *oid)
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

bool snmpoid_start_by(const char *begin, const char *str)
{
    int i;

    i=0;
    while(begin[i] != '\0')
    {
        // no end of string control is necessary for parameter 'str'
        // because if str[i] == '\0', the next condition will be true
        if (begin[i] != str[i])
        {
            return false;
        }
        i++;
    }

    return TRUE;
}

bool snmpoid_valid(const char *oid)
{
    bool dot;

    dot = true; // first character must be a digit
    while(*oid != '\0')
    {
        if (*oid == OID_SEPARATOR)
        {
            if (dot) // double dot
            {
                return false;
            }
            dot = true;
        }
        else if (*oid >= '0' && *oid <= '9' )
        {
            dot = false;
        }
        else
        {
            return false;
        }
        oid++;
    }

	return (!dot); // one loop is mandatory and last character must not be a dot
}

void snmptrap_gets(char *buffer, snmpTrap *trap)
{
        char *oid, *value;

        oid = strtok(buffer, "\t\n");
        value = strtok(NULL, "\t\n");

        if (snmpoid_valid(oid) != TRUE)
        {
            // not-valid oid
            return;
        }

        if (snmpoid_start_by(OID_COMMUNITY, oid))
        {
            strncpy(trap->community, value, sizeof(trap->community));
        }
        else if (snmpoid_start_by(OID_TIMESTAMP, oid))
        {
            sscanf(value, "%u", &trap->timestamp);
        }
        else if (snmpoid_start_by(OID_AGENT, oid))
        {
            strncpy(trap->agent, value, sizeof(trap->agent));
        }
        else if (snmpoid_start_by(OID_ENTERPRISE, oid))
        {
            strncpy(trap->enterprise, value, sizeof(trap->enterprise));
        }
        else if (snmpoid_start_by(OID_TRAP_CODE, oid))
        {
            if (snmpoid_start_by(OID_TRAP_CODE_GENERIC, value))
            {
                // warning: 1 is added to last OID for generic trap to avoid '0' value
                trap->generic_type = snmpoid_last_id(value) - 1;
                trap->specific_type = 0;
            }
            else
            {
                trap->generic_type = SPECIFIC_TYPE_GENERIC;
                trap->specific_type = snmpoid_last_id(value);
            }
        }
        else
        {
            trap->variables_oid[trap->variables_count] = malloc(strlen(oid)+1);
            strncpy(trap->variables_oid[trap->variables_count], oid, (strlen(oid)+1));

            trap->variables_value[trap->variables_count] = malloc(strlen(value)+1);
            strncpy(trap->variables_value[trap->variables_count], value, (strlen(value)+1));

            trap->variables_count++;
        }
}


const char *snmptrap_eventname(snmpTrap *trap)
{
	static const char *trapname[] = { "cold start", "warm start", "link down", "link up",
										"authentication failure", "routing table updated", "enterprise specific alert" };

    if (trap->generic_type >= 0 && trap->generic_type <= 6)
    {
        return trapname[trap->generic_type];
    }
    else
    {
        return "unknow";
    }
}

unsigned int snmptrap_code(snmpTrap *trap)
{
    return (trap->generic_type == 6 ? trap->specific_type : trap->generic_type);
}

const char *snmptrap_description(snmpTrap *trap)
{
	static const char *trapdesc[] =
	{
		"Agent %s has started.",
		"Agent %s has restared, maybe due to a configuration change.",
		"Network interface of agent %s have been change its state to down.",
		"Network interface of agent %s have been change its state to up.",
		"Unauthorised person attempted to access to the agent %s.",
		"The routing table of agent %s has been updated.",
		"Specific event of manufacturer of agent %s."
	};
    static char description_buffer[256];

    if (trap->generic_type >= 0 && trap->generic_type <= 6)
    {
        snprintf(description_buffer, sizeof(description_buffer), trapdesc[trap->generic_type], trap->agent);
        return description_buffer;
    }
    else
    {
        return "";
    }
}
