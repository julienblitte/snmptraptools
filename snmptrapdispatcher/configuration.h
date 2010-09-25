#include "trapSnmp.h"

#ifndef H_CONFIGURATION_INCLUDED
#define H_CONFIGURATION_INCLUDED

typedef struct _trap_action_entry
{
    char *oid;
    long specificType;
    long genericType;
    char *run;
    char *wkDir;
} trap_action_entry;

trap_action_entry *configurationLoad();
void configurationClean(trap_action_entry *list);
void configurationMatch(trap_action_entry *list, snmpTrap *trap,
        void (actionCallback)(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber));

#endif
