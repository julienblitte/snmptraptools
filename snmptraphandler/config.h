#define SERVICE_NAME "snmpTrapHandler";
#define SERVICE_DESC "SnmpTrapHandler service provides a gateway to translate recieved alert to an external program";

#define DISPATCHER "snmptrapdispatcher.exe"
#define MAX_DISPATCHER_LINE_LEN 4096

#define ERROR_SNMP_INIT         1
#define ERROR_DISPATCHER_INIT   2
#define ERROR_DISPATCHER_WRITE  3

#define LOG_FILENAME "snmptraphandler.log"



