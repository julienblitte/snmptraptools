#include <winsnmp.h>
#include <stdint.h>
#include "snmp.h"
#include "..\logger.h"
#include "..\..\core\trapSnmp.h"

SNMPAPI_STATUS SnmpValueToStr(smiLPVALUE value, char *destBuffer, size_t lenBuffer)
{
    unsigned int i;
    ULARGE_INTEGER u64;

    switch(value->syntax)
    {
        case SNMP_SYNTAX_INT32:
            snprintf(destBuffer, lenBuffer, "%d", value->value.sNumber);
            break;

        case SNMP_SYNTAX_OCTETS:
        case SNMP_SYNTAX_OPAQUE:
        case SNMP_SYNTAX_BITS:
            for(i = 0; (i < value->value.string.len) && (i < (lenBuffer-1)); i++)
            {
                if ((unsigned)value->value.string.ptr[i] >= 32)
                {
                    destBuffer[i] = value->value.string.ptr[i];
                }
                else
                {
                    if ((value->value.string.ptr[i] == '\0') && (i != value->value.string.len))
                    {
                        break;
                    }
                    destBuffer[i] = '.';
                }
            }

            if (i <= lenBuffer)
            {
                destBuffer[i] = '\0';
            }
            break;

        case SNMP_SYNTAX_NULL:
            snprintf(destBuffer, lenBuffer, "null");
            break;

        case SNMP_SYNTAX_OID:
            if (SnmpOidToStr(&value->value.oid, lenBuffer, destBuffer) == SNMPAPI_FAILURE)
            {
                logPrintf(LOG_ERROR, "SnmpOidToStr(SnmpValueToStr) failed!\n");
                return SNMPAPI_FAILURE;
            }
            break;

        case SNMP_SYNTAX_IPADDR:
            switch (value->value.string.len)
            {
                case 4:
                    snprintf(destBuffer, lenBuffer, "%u.%u.%u.%u",
                             ((unsigned char)value->value.string.ptr[0]),
                             ((unsigned char)value->value.string.ptr[1]),
                             ((unsigned char)value->value.string.ptr[2]),
                             ((unsigned char)value->value.string.ptr[3])
                    );
                    break;
                case 16:
                    snprintf(destBuffer, lenBuffer, "%x:%x:%x:%x:%x:%x:%x:%x",
							 ntohs(((uint16_t *)value->value.string.ptr)[0]),
							 ntohs(((uint16_t *)value->value.string.ptr)[1]),
							 ntohs(((uint16_t *)value->value.string.ptr)[2]),
							 ntohs(((uint16_t *)value->value.string.ptr)[3]),
							 ntohs(((uint16_t *)value->value.string.ptr)[4]),
							 ntohs(((uint16_t *)value->value.string.ptr)[5]),
							 ntohs(((uint16_t *)value->value.string.ptr)[6]),
							 ntohs(((uint16_t *)value->value.string.ptr)[7])
/*
                             (((unsigned short)value->value.string.ptr[0]) << 8) | (((unsigned short)value->value.string.ptr[1]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[2]) << 8) | (((unsigned short)value->value.string.ptr[3]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[4]) << 8) | (((unsigned short)value->value.string.ptr[5]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[6]) << 8) | (((unsigned short)value->value.string.ptr[7]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[8]) << 8) | (((unsigned short)value->value.string.ptr[9]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[10]) << 8) | (((unsigned short)value->value.string.ptr[11]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[12]) << 8) | (((unsigned short)value->value.string.ptr[13]) & 0xFF),
                             (((unsigned short)value->value.string.ptr[14]) << 8) | (((unsigned short)value->value.string.ptr[15]) & 0xFF)
*/
                    );
                    break;
            }
            break;

        case SNMP_SYNTAX_CNTR32:
        case SNMP_SYNTAX_GAUGE32:
        case SNMP_SYNTAX_TIMETICKS:
        case SNMP_SYNTAX_UINT32:
            snprintf(destBuffer, lenBuffer, "%u", value->value.uNumber);
            break;

        case SNMP_SYNTAX_CNTR64:
            u64.HighPart = value->value.hNumber.hipart;
            u64.LowPart = value->value.hNumber.lopart;
            wsprintf(destBuffer, "%I64u", u64);
            break;

        case SNMP_SYNTAX_NOSUCHOBJECT:
        case SNMP_SYNTAX_NOSUCHINSTANCE:
        case SNMP_SYNTAX_ENDOFMIBVIEW:
            snprintf(destBuffer, lenBuffer, "!");
            return SNMPAPI_FAILURE;
    }

    return SNMPAPI_SUCCESS;
}

SNMPAPI_STATUS SnmpFreeValue(smiLPVALUE value)
{
    switch(value->syntax)
    {
        case SNMP_SYNTAX_OID:
            return SnmpFreeDescriptor(SNMP_SYNTAX_OID, (smiLPOPAQUE)&value->value);
        case SNMP_SYNTAX_OCTETS:
             return SnmpFreeDescriptor(SNMP_SYNTAX_OCTETS, (smiLPOPAQUE)&value->value);
    }

    return SNMPAPI_SUCCESS;
}
