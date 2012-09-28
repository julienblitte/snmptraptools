#ifndef H_CONFIGURATION_INCLUDED
#define H_CONFIGURATION_INCLUDED

#include "..\core\trapSnmp.h"

typedef struct _trap_action_entry
{
    char *oid;
    uint32_t specificType;
    uint32_t genericType;
	uint32_t pluginUID;
    char *desc;
} trap_action_entry;

trap_action_entry *configurationLoad();
void configurationClean(trap_action_entry *list);
void configurationMatch(trap_action_entry *list, snmpTrap *trap,
        void (actionCallback)(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber));

#endif
