#ifndef H_TRAPSNMP_INCLUDED
#define H_TRAPSNMP_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include "snmptraptools_config.h"

#ifndef ntohs
	#define ntohs(W)	((((uint16_t)W)<<8)|(((uint16_t)W)>>8))
#endif

typedef struct
{
    time_t date;
    char community[MAX_COMMUNITY_LEN];
    uint32_t timestamp;
    uint32_t generic_type;
    uint32_t specific_type;
    char agent[MAX_NETWORK_ADDRESS_LEN];
    char enterprise[MAX_OID_LEN];
    uint32_t variables_count;
    char *variables_oid[MAX_TRAP_VARIABLES];
    char *variables_value[MAX_TRAP_VARIABLES];
} snmpTrap;

void snmptrap_free(snmpTrap *trap);
void snmptrap_print(FILE *out, snmpTrap *trap);
long snmpoid_last_id(const char *oid);
bool snmpoid_start_by(const char *begin, const char *str);
bool snmpoid_valid(const char *oid);
void snmptrap_gets(char *buffer, snmpTrap *trap);

const char *snmptrap_eventname(snmpTrap *trap);
unsigned int snmptrap_code(snmpTrap *trap);
const char *snmptrap_description(snmpTrap *trap);

#endif
