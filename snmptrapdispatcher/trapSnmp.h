#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "../snmptraptool_config.h"

#ifndef H_TRAPSNMP_INCLUDED
#define H_TRAPSNMP_INCLUDED

typedef struct
{
    time_t date;
    char community[MAX_COMMUNITY_LEN];
    unsigned long int timestamp;
    long genericTrap;
    long specificTrap;
    char agent[MAX_NETWORK_ADDRESS_LEN];
    char enterprise[MAX_OID_LEN];
    unsigned long int variablesCount;
    char *variablesOID[MAX_TRAP_VARIABLES];
    char *variablesValue[MAX_TRAP_VARIABLES];
} snmpTrap;

void trapFree(snmpTrap *trap);
void trapPrint(FILE *out, snmpTrap *trap);
long oidLastId(const char *oid);
bool oidStartBy(const char *begin, const char *str);
bool oidIsValid(const char *oid);
void trapReadLine(char *buffer, snmpTrap *trap);

#endif
