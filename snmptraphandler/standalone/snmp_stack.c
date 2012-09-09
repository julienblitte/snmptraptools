#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "..\logger.h"
#include "..\dispatcher.h"
#include "..\..\core\trapSnmp.h"

#define SNMP_SYNTAX_INT			0x02
#define SNMP_SYNTAX_BITS		0x03
#define SNMP_SYNTAX_OCTETS		0x04
#define SNMP_SYNTAX_BITS		0x03
#define SNMP_SYNTAX_NULL		0x05
#define SNMP_SYNTAX_OID			0x06
#define SNMP_SYNTAX_SEQUENCE	0x30
#define SNMP_SYNTAX_IPADDR		0x40
#define SNMP_SYNTAX_CNTR32		0x41
#define SNMP_SYNTAX_GAUGE32		0x42
#define SNMP_SYNTAX_TIMETICKS	0x43
#define SNMP_SYNTAX_OPAQUE		0x44
#define SNMP_SYNTAX_UINT32		0x47
#define SNMP_SYNTAX_CNTR64		0x46

#define SNMP_SYNTAX_TRAP        0xA4

bool snmp_value2str(const char *value, size_t value_len, char *buffer, size_t buffer_len, char value_type);

unsigned long snmp_length(const char **buffer, unsigned int *buffer_len)
{
        unsigned long result;
        unsigned int length;

        if (*buffer_len <= 0)
        {
            return 0;
        }

        if ((**buffer & 0x80) == 0)
        {
            result = **buffer & 0xFF;
            (*buffer)++, (*buffer_len)--;
        }
        else
        {
			// length of "real length" value
            length = **buffer & 0x7F;
            (*buffer)++, (*buffer_len)--;

            result = 0;
            while ((length != 0) && (buffer_len != 0))
            {
                result <<= 8;
                result |= **buffer & 0xFF;

                (*buffer)++, (*buffer_len)--;
                length--;
            }
        }

        return result;
}

bool snmp_oid2str(const char *value, size_t value_len, char *buffer, size_t buffer_len)
{
    unsigned int i;
    char *cat_buffer;
    unsigned int multibyte;

    if (*value < 1)
    {
        return false;
    }

    cat_buffer = (char *)malloc(buffer_len);
    if (cat_buffer == NULL)
    {
        return false;
    }

    // first byte is strangely encoded!?
    snprintf(buffer, buffer_len, "%u%c%u", value[0]/40, OID_SEPARATOR, value[0]%40);

    i = 1, multibyte = 0;
    while(i < value_len)
    {
        // long number encoding
        if ((value[i] & 0x80) != 0)
        {
            multibyte <<= 7;
            multibyte |= value[i] & 0x7F;
        }
        else if (multibyte != 0)
        {
            multibyte <<= 7;
            multibyte |= value[i] & 0x7F;

            memcpy(cat_buffer, buffer, buffer_len);
            snprintf(buffer, buffer_len, "%s%c%u", cat_buffer, OID_SEPARATOR, multibyte);
            // force value to zero to indicate that next byte is not part of a multi-byte coded value
            multibyte = 0;
        }
        else
        {
            memcpy(cat_buffer, buffer, buffer_len);
            snprintf(buffer, buffer_len, "%s%c%u", cat_buffer, OID_SEPARATOR, value[i]);
        }
        i++;
    }

    free(cat_buffer);

    return true;
}

bool snmp_read_value(const char **buffer, unsigned int *buffer_len,
                        char *dest_buffer, char dest_type, unsigned int dest_len)
{
    char read_type;
    unsigned long read_len;

    // do not go out of buffer
    if (*buffer_len < 2)
    {
        return false;
    }

    // read type and length
    read_type = **buffer;

    (*buffer)++;
    (*buffer_len)--;

    read_len = snmp_length(buffer, buffer_len);

    // do not go out of buffer
    if (*buffer_len <  read_len)
    {
        logPrintf(LOG_ERROR, "protocol error: data size seems invalid!\n");
        return false;
    }

    // check wanted variable type
    if (read_type != dest_type && dest_type != SNMP_SYNTAX_OPAQUE)
    {
        return false;
    }

    // check destination buffer size
    if (dest_len < read_len)
    {
        return false;
    }

    if (!snmp_value2str(*buffer, read_len, dest_buffer, dest_len, read_type))
    {
        return false;
    }

    *buffer += read_len;
    *buffer_len -= read_len;

    return true;
}

bool snmp_open_sequence(const char **buffer, unsigned int *buffer_len, char sequence_type)
{
    char read_type;
    unsigned long read_len;

    // do not go out of buffer
    if (*buffer_len < 2)
    {
        return false;
    }

    // read type and length
    read_type = **buffer;

    (*buffer)++;
    (*buffer_len)--;

    read_len = snmp_length(buffer, buffer_len);

    // do not go out of buffer
    if (*buffer_len <  read_len)
    {
        logPrintf(LOG_ERROR, "protocol error: data size seems invalid!\n");
        return false;
    }

    // check if sequence goes out of buffer
    if (*buffer_len <  read_len)
    {
        logPrintf(LOG_ERROR, "protocol error: sequence size seems invalid!\n");
        logPrintf(LOG_ERROR, "\texcepted size:%u, size of data:%u\n", *buffer_len, read_len);
        return false;
    }

    // check if it's a sequence
    if (read_type != sequence_type)
    {
        return false;
    }

    return true;
}

void snmp_trap_decode(const char *data, const unsigned int data_len)
{
    const char *local_addr;
    unsigned int local_len;
    unsigned int trap_type;

    char buffer[1024];
    char entreprise_oid[1024];

    local_addr = data;
    local_len = data_len;

    // global sequence
    if (!snmp_open_sequence(&local_addr, &local_len, SNMP_SYNTAX_SEQUENCE))
    {
        logPrintf(LOG_ERROR, "protocol error: failed opening a sequence (first)\n");
        return;
    }

    // snmp version integer
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_INT, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading integer (version)\n");
        return;
    }
    if (strcmp(buffer, "0") != 0)
    {
        logPrintf(LOG_ERROR, "protocol error: bad snmp version\n");
        return;
    }

    // community string
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_OCTETS, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading string (community)\n");
        return;
    }
    else
    {
        printDispatcherF("%s\t%s\n", OID_COMMUNITY, buffer);
    }

    // trap sequence
    if (!snmp_open_sequence(&local_addr, &local_len, SNMP_SYNTAX_TRAP))
    {
            logPrintf(LOG_ERROR, "protocol error: failed reading string (trap)\n");
            printDispatcherF("\n");
            return;
    }

    // oid
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_OID, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading oid (trap)\n");
        printDispatcherF("\n");
        return;
    }
    else
    {
        strncpy(entreprise_oid, buffer, sizeof(entreprise_oid));
        printDispatcherF("%s\t%s\n", OID_ENTERPRISE, buffer);
    }

    // source ip
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_IPADDR, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading ip (agent)\n");
        printDispatcherF("\n");
        return;
    }
    else
    {
        printDispatcherF("%s\t%s\n", OID_AGENT, buffer);
    }

    // generic trap type
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_INT, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading integer (generic trap code)\n");
        printDispatcherF("\n");
        return;
    }

    trap_type = atoi(buffer);
    if (trap_type != SPECIFIC_TYPE_GENERIC)
    {
        // generic type
        printDispatcherF("%s\t%s%c%u\n", OID_TRAP_CODE, OID_TRAP_CODE_GENERIC, OID_SEPARATOR, trap_type+1);
    }

    // enterprise trap type
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_INT, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading integer (specific trap code)\n");
        printDispatcherF("\n");
        return;
    }

    if (trap_type == SPECIFIC_TYPE_GENERIC)
    {
        // specific type
        trap_type = atoi(buffer);
        printDispatcherF("%s\t%s%c%u\n", OID_TRAP_CODE, entreprise_oid, OID_SEPARATOR, trap_type);
    }

    // timestamp
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_TIMETICKS, sizeof(buffer)))
    {
        logPrintf(LOG_ERROR, "protocol error: failed reading timestamp (date)\n");
        printDispatcherF("\n");
        return;
    }
    else
    {
        printDispatcherF("%s\t%s\n", OID_TIMESTAMP, buffer);
    }

    // variables sequence
    if (!snmp_open_sequence((const char**)&local_addr, &local_len, SNMP_SYNTAX_SEQUENCE))
    {
            logPrintf(LOG_ERROR, "protocol error: variables sequence invalid!\n");
            return;
    }

    while(local_len > 0)
    {
        if (!snmp_open_sequence((const char **)&local_addr, &local_len, SNMP_SYNTAX_SEQUENCE))
        {
            printDispatcherF("\n");
            return;
        }

        if (!snmp_read_value((const char **)&local_addr, &local_len, buffer, SNMP_SYNTAX_OID, sizeof(buffer)))
        {
            printDispatcherF("\n");
            return;
        }
        printDispatcherF("%s\t", buffer);

        if (!snmp_read_value((const char **)&local_addr, &local_len, buffer, SNMP_SYNTAX_OPAQUE, sizeof(buffer)))
        {
            printDispatcherF("-\n");
            return;
        }
        printDispatcherF("%s\n", buffer);
    }

    printDispatcherF("\n");
}

bool snmp_value2str(const char *value, size_t value_len, char *buffer, size_t buffer_len, char value_type)
{
    unsigned int i;
    ULARGE_INTEGER u64;

    i = 0;
    switch(value_type)
    {
        case SNMP_SYNTAX_INT:
			if (value_len == 0)
			{
				break;
			}
			// take care about sign bit propagation
			u64.LowPart = (value[0] & 0x80 ? (unsigned)-1 : 0);
            for(i=0; (i < value_len) && (i < sizeof(u64.LowPart)); i++)
            {
                u64.LowPart <<= 8;
                u64.LowPart |= (value[i] & 0x000000ff);
            }
            snprintf(buffer, buffer_len, "%d", (int)u64.LowPart);
            break;

        case SNMP_SYNTAX_OCTETS:
        case SNMP_SYNTAX_OPAQUE:
        case SNMP_SYNTAX_BITS:
            for(i = 0; (i < value_len) && (i < (buffer_len-1)); i++)
            {
                if ((unsigned)value[i] >= 32)
                {
                    buffer[i] = value[i];
                }
                else
                {
                    if ((value[i] == '\0') && (i != value_len))
                    {
                        break;
                    }
                    buffer[i] = '.';
                }
            }

            if (i <= buffer_len)
            {
                buffer[i] = '\0';
            }
            else
            {
                buffer[buffer_len] = '\0';
            }
            break;

        case SNMP_SYNTAX_NULL:
            snprintf(buffer, buffer_len, "null");
            break;

        case SNMP_SYNTAX_OID:
            snmp_oid2str(value, value_len, buffer, buffer_len);
            break;

        case SNMP_SYNTAX_IPADDR:
            switch (value_len)
            {
                case 4:
                    snprintf(buffer, buffer_len, "%u.%u.%u.%u",
                             ((uint8_t)value[0]),
                             ((uint8_t)value[1]),
                             ((uint8_t)value[2]),
                             ((uint8_t)value[3])
                    );
                    break;
                case 16:
                    snprintf(buffer, buffer_len, "%x:%x:%x:%x:%x:%x:%x:%x",
							 ntohs(((uint16_t *)value)[0]),
							 ntohs(((uint16_t *)value)[1]),
							 ntohs(((uint16_t *)value)[2]),
							 ntohs(((uint16_t *)value)[3]),
							 ntohs(((uint16_t *)value)[4]),
							 ntohs(((uint16_t *)value)[5]),
							 ntohs(((uint16_t *)value)[6]),
							 ntohs(((uint16_t *)value)[7])
/*
							 (((uint16_t)value[0]) << 8) | (((uint16_t)value[1]) & 0xFF),
                             (((uint16_t)value[2]) << 8) | (((uint16_t)value[3]) & 0xFF),
                             (((uint16_t)value[4]) << 8) | (((uint16_t)value[5]) & 0xFF),
                             (((uint16_t)value[6]) << 8) | (((uint16_t)value[7]) & 0xFF),
                             (((uint16_t)value[8]) << 8) | (((uint16_t)value[9]) & 0xFF),
                             (((uint16_t)value[10]) << 8) | (((uint16_t)value[11]) & 0xFF),
                             (((uint16_t)value[12]) << 8) | (((uint16_t)value[13]) & 0xFF),
                             (((uint16_t)value[14]) << 8) | (((uint16_t)value[15]) & 0xFF)
*/
                    );
                    break;
                default:
                    strncpy(buffer, "", buffer_len);
            }
            break;

        case SNMP_SYNTAX_CNTR32:
        case SNMP_SYNTAX_GAUGE32:
        case SNMP_SYNTAX_TIMETICKS:
        case SNMP_SYNTAX_UINT32:
			u64.LowPart = 0;
			// we don't know the length
            for(i=0; (i < value_len) && (i < sizeof(u64.LowPart)); i++)
            {
                u64.LowPart <<= 8;
                u64.LowPart |= (value[i] & 0x000000FF);
            }
            snprintf(buffer, buffer_len, "%u", (uint32_t)u64.LowPart);
            break;

        case SNMP_SYNTAX_CNTR64:
			// big endian
            memcpy(&u64.HighPart, value, 4);
            memcpy(&u64.LowPart, value+4, 4);
            wsprintf(buffer, "%I64u", u64);
            break;

        default:
            return false;
    }

    return true;
}
