#ifndef H_TRAPSNMP_INCLUDED
#define H_TRAPSNMP_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define MAX_COMMUNITY_LEN           256
#define MAX_NETWORK_ADDRESS_LEN     64
#define MAX_TRAP_VARIABLES          256

#define OID_COMMUNITY   "1.3.6.1.6.3.1.2.2.9"
#define OID_TIMESTAMP   "1.3.6.1.2.1.1.3"
#define OID_AGENT       "1.3.6.1.3.1057.1"
#define OID_ENTERPRISE  "1.3.6.1.6.3.1.1.4.3"
#define OID_TRAP_CODE   "1.3.6.1.6.3.1.1.4.1"
#define OID_TRAP_CODE_GENERIC   "1.3.6.1.6.3.1.1.5"

#define MAX_INPUT_LINE_LEN          4096

#define OID_SEPARATOR               '.'
#define SPECIFIC_TYPE_GENERIC       6
#define TRAP_TYPE_UNKNOW            -1

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
BOOL oidStartBy(const char *begin, const char *str);
BOOL oidIsValid(const char *oid);
void trapReadLine(char *buffer, snmpTrap *trap);

#endif
