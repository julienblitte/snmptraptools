#include <winsnmp.h>

#define SnmpFreeOID(oid)    SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)(oid))
#define SnmpFreeOCTETS(octets)  SnmpFreeDescriptor(SNMP_SYNTAX_OCTETS, (smiLPOPAQUE)(octets))

SNMPAPI_STATUS SnmpValueToStr(smiLPVALUE value, char *destBuffer, size_t lenBuffer);
SNMPAPI_STATUS SnmpFreeValue(smiLPVALUE value);
