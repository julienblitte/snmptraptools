#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>
#include "configuration.h"
#include "../snmptraptools_config.h"
#include "../libtrapsnmp/trapSnmp.h"
#include "../libregistry/registry.h"

trap_action_entry *configurationLoad()
{
    HKEY hKey;
    DWORD keyIndex, keyCount;
    char buffer[MAX_REGISTRY_LEN];
	char currentKey[MAX_REGISTRY_LEN];
    DWORD bufferSize;

    trap_action_entry *result;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        printf("Unable to access key in registry!\n");
        return NULL;
    }

    if (RegCountSubKeys(hKey, &keyCount) != ERROR_SUCCESS)
    {
        printf("RegCountSubKeys failed!\n");
        return NULL;
    }

    result = calloc(keyCount+1, sizeof(trap_action_entry));
    if (result == NULL)
    {
        printf("calloc failed!\n");
        return NULL;
    }

    // TODO: try to read allowed communit(y|ies) here

    keyIndex = 0;
    bufferSize = sizeof(currentKey);
    while((RegEnumKeyEx(hKey, keyIndex, currentKey, &bufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
          && (keyIndex < keyCount))
    {
        result[keyIndex].specificType = TRAP_TYPE_UNKNOW;
        result[keyIndex].genericType = TRAP_TYPE_UNKNOW;

        bufferSize = sizeof(buffer);
        if (RegGetValue(hKey, currentKey, REGISTRY_OID, 0, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
        {
            result[keyIndex].oid = strdup(buffer);
        }
		else
		{
			// for configuration of previous versions
			// oid = name of key
			result[keyIndex].oid = strdup(currentKey);
		}

		bufferSize = sizeof(buffer);
		if (RegGetValue(hKey, currentKey, REGISTRY_SPECIFIC_TYPE, 0, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			// result[keyIndex].specificType = *((long *)buffer);
			memcpy(&result[keyIndex].specificType, buffer, sizeof(((trap_action_entry*)NULL)->specificType));
		}

		bufferSize = sizeof(buffer);
		if (RegGetValue(hKey, currentKey, REGISTRY_GENERIC_TYPE, 0, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			//result[keyIndex].genericType = *((long *)buffer);
			memcpy(&result[keyIndex].genericType, buffer, sizeof(((trap_action_entry*)NULL)->genericType));
		}

		bufferSize = sizeof(buffer);
		if (RegGetValue(hKey, currentKey, REGISTRY_RUN, 0, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			result[keyIndex].run = strdup(buffer);
		}

		bufferSize = sizeof(buffer);
		if (RegGetValue(hKey, currentKey, REGISTRY_WKDIR, 0, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
		{
			result[keyIndex].wkDir = strdup(buffer);
		}

        if ((result[keyIndex].genericType == TRAP_TYPE_UNKNOW) && (result[keyIndex].specificType != TRAP_TYPE_UNKNOW))
        {
            result[keyIndex].genericType = SPECIFIC_TYPE_GENERIC;
        }

        bufferSize = sizeof(currentKey); keyIndex++;
    }

    RegCloseKey(hKey);

    return result;
}

void configurationClean(trap_action_entry *list)
{
    unsigned long i;

    i = 0;
    while(list[i].oid != NULL)
    {
        free(list[i].oid);
        if (list[i].run != NULL)
        {
            free(list[i].run);
        }
        if (list[i].wkDir != NULL)
        {
            free(list[i].wkDir);
        }

        i++;
    }

    free(list);
}

void configurationMatch(trap_action_entry *list, snmpTrap *trap,
        void (actionCallback)(trap_action_entry *action, snmpTrap *trap, unsigned long actionNumber))
{
    unsigned long i;

    i = 0;
    while(list[i].oid != NULL)
    {
        if (oidStartBy(list[i].oid, trap->enterprise) == true)
        {
            if ((list[i].genericType != TRAP_TYPE_UNKNOW)
                && (list[i].genericType != trap->genericTrap))
            {
                i++;
                continue;
            }

            if (((list[i].genericType == SPECIFIC_TYPE_GENERIC) || (list[i].genericType == TRAP_TYPE_UNKNOW))
               && (list[i].specificType != TRAP_TYPE_UNKNOW) && (list[i].specificType != trap->specificTrap))
            {
                i++;
                continue;
            }

            actionCallback(&list[i], trap, i);
        }
        i++;
    }
}
