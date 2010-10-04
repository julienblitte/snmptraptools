#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../logger.h"
#include "../dispatcher.h"
#include "../../libtrapsnmp/trapSnmp.h"

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
            // force value to zero to indicate that previous byte was not multi-byte coded
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
    char read_len;

    // do not go out of buffer
    if (*buffer_len < 2)
    {
        return false;
    }

    // read type and length
    read_type = (*buffer)[0];
    read_len = (*buffer)[1];

    *buffer += 2;
    *buffer_len -= 2;

    // do not go out of buffer
    if (*buffer_len <  read_len)
    {
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
    char read_len;

    // do not go out of buffer
    if (*buffer_len < 2)
    {
        return false;
    }

    // read type and length
    read_type = (*buffer)[0];
    read_len = (*buffer)[1];

    // check if sequence goes out of buffer
    if (*buffer_len <  2 + read_len)
    {
        return false;
    }

    // check if it's a sequence
    if (read_type != sequence_type)
    {
        return false;
    }

    *buffer += 2;
    *buffer_len -= 2;

    return true;
}

//TODO: replace printf by printDispatcher
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
        return;
    }

    // snmp version integer
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_INT, sizeof(buffer)))
    {
        return;
    }
    if (strcmp(buffer, "0") != 0)
    {
        return;
    }

    // community string
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_OCTETS, sizeof(buffer)))
    {
        return;
    }
    else
    {
        printDispatcherF("%s\t%s\n", OID_COMMUNITY, buffer);
    }

    // trap sequence
    if (!snmp_open_sequence(&local_addr, &local_len, SNMP_SYNTAX_TRAP))
    {
            printDispatcherF("\n");
            return;
    }

    // oid
    if (!snmp_read_value(&local_addr, &local_len, buffer, SNMP_SYNTAX_OID, sizeof(buffer)))
    {
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
            if (value_len > 0 && value_len <= sizeof(i))
            {
                memcpy(&i, value, value_len);
                i &= (0xFFFFFFFF >> 8 * (sizeof(i) - value_len));
            }
            snprintf(buffer, buffer_len, "%d", i);
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
                             ((unsigned char)value[0]),
                             ((unsigned char)value[1]),
                             ((unsigned char)value[2]),
                             ((unsigned char)value[3])
                    );
                    break;
                case 16:
                    snprintf(buffer, buffer_len, "%x:%x:%x:%x:%x:%x:%x:%x",
                             (((unsigned short)value[0]) << 8) | (((unsigned short)value[1]) & 0xFF),
                             (((unsigned short)value[2]) << 8) | (((unsigned short)value[3]) & 0xFF),
                             (((unsigned short)value[4]) << 8) | (((unsigned short)value[5]) & 0xFF),
                             (((unsigned short)value[6]) << 8) | (((unsigned short)value[7]) & 0xFF),
                             (((unsigned short)value[8]) << 8) | (((unsigned short)value[9]) & 0xFF),
                             (((unsigned short)value[10]) << 8) | (((unsigned short)value[11]) & 0xFF),
                             (((unsigned short)value[12]) << 8) | (((unsigned short)value[13]) & 0xFF),
                             (((unsigned short)value[14]) << 8) | (((unsigned short)value[15]) & 0xFF)
                    );
                    break;
            }
            break;

        case SNMP_SYNTAX_CNTR32:
        case SNMP_SYNTAX_GAUGE32:
        case SNMP_SYNTAX_TIMETICKS:
        case SNMP_SYNTAX_UINT32:
            snprintf(buffer, buffer_len, "%u", *value);
            break;

        case SNMP_SYNTAX_CNTR64:
            memcpy(&u64.HighPart, value, 4);
            memcpy(&u64.LowPart, value+4, 4);
            wsprintf(buffer, "%I64u", u64);
            break;

        default:
            return false;
    }

    return true;
}
