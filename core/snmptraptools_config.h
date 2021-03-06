#ifndef SNMPTRAPTOOL_CONFIG_H_INCLUDED
#define SNMPTRAPTOOL_CONFIG_H_INCLUDED

#define SERVICE_NAME		"snmptraptools"
#define SERVICE_DESC		"provides a gateway to translate recieved SNMP trap to an external program"

#define SERVER_LISTEN_PORT			162
#define REGISTRY_SERVICE_PATH		"SYSTEM\\CurrentControlSet\\Services\\snmptraptools\\"
#define REGISTRY_ROOT_PATH			"SYSTEM\\CurrentControlSet\\Services\\snmptraptools\\"
#define REGISTRY_CONFIG_PATH		"SYSTEM\\CurrentControlSet\\Services\\snmptraptools\\dispatcher\\"
#define REGISTRY_PLUGIN_CONFIG_PATH		"SYSTEM\\CurrentControlSet\\Services\\snmptraptools\\plugins\\"
#define REGISTRY_SERVICE_EXECUTABLE		"ImagePath"
#define REGISTRY_OID			NULL
#define REGISTRY_PLUGIN         "plugin"
#define REGISTRY_SPECIFIC_TYPE  "specificType"
#define REGISTRY_GENERIC_TYPE   "genericType"
#define REGISTRY_DESCRIPTION    "description"


#define OID_SEPARATOR               '.'
#define SPECIFIC_TYPE_GENERIC       6
#define TRAP_TYPE_UNKNOW            -1
#define DISPATCHER					"snmptrapdispatcher.exe"

#define LOG_ERROR       4
#define LOG_WARNING     3
#define LOG_INFORMATION 2
#define LOG_DEBUG       1
#define LOG_PEDANTIC    0
#define LOG_FILENAME				"snmptrapservice.log"

#define DISTPATCHER_LOGFILE			"snmptrapnetwork.log"

#define MAX_DISPATCHER_LINE_LEN		4096
#define MAX_PARAMETERS              2048
#define MAX_DESCRIPTION_LEN         1024
#define MAX_COMMUNITY_LEN			256
#define MAX_NETWORK_ADDRESS_LEN     64
#define MAX_OID_LEN                 256
#define MAX_TRAP_VARIABLES          256
#define MAX_REGISTRY_LEN            512
#define MAX_PLUGINS					256
#define MAX_PLUGIN_CONFIG_LEN		4096

#define ERROR_SNMP_INIT         1
#define ERROR_DISPATCHER_INIT   2
#define ERROR_DISPATCHER_WRITE  3
#define ERROR_THREAD_INIT       4

#define OID_COMMUNITY   "1.3.6.1.6.3.1.2.2.9.0"
#define OID_TIMESTAMP   "1.3.6.1.2.1.1.3.0"
#define OID_AGENT       "1.3.6.1.3.1057.1"
#define OID_ENTERPRISE  "1.3.6.1.6.3.1.1.4.3.0"
#define OID_TRAP_CODE   "1.3.6.1.6.3.1.1.4.1.0"
#define OID_TRAP_CODE_GENERIC   "1.3.6.1.6.3.1.1.5.0"

#define DEFAULT_PLUGIN  0

#define PLUGIN_FILTER	"plugin_*.dll"

#define VERSION_NUM		0,3,0,1
#define VERSION_STR		"0,3,0,1a\0"
#define VERSION_BUILD	"Built the 2012-09-30 on Estephe with MinGW tool set + bison fix\0"

#endif // SNMPTRAPTOOL_CONFIG_H_INCLUDED
