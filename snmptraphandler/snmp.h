#include <winsnmp.h>

SNMPAPI_STATUS SnmpValueToStr(smiLPVALUE value, char *destBuffer, size_t lenBuffer);
SNMPAPI_STATUS SnmpFreeOID(smiLPOID oid);
SNMPAPI_STATUS SnmpFreeOCTETS(smiLPOCTETS octets);
SNMPAPI_STATUS SnmpFreeValue(smiLPVALUE value);
