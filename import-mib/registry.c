#include <windows.h>
#include "registry.h"
#include "..\core\snmptraptools_config.h"
#include "..\libregistry\registry.h"

HKEY registryOpen()
{
	HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return NULL;
    }

	return hKey;
}

BOOL registryAddOid(HKEY hKey, LPCTSTR oid, DWORD trapCode, LPCTSTR description)
{
    HKEY  hSubKey;
    BOOL result;

    // add a new subkey
    if (RegCreateSubKey(hKey, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, &hSubKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    result = TRUE;

    // add value REGISTRY_OID in this key
    if (RegSetValueEx(hSubKey, REGISTRY_OID, 0, REG_SZ, (BYTE*)oid, strlen(oid)+1) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

	// add value REGISTRY_SPECIFIC_TYPE
    if (RegSetValueEx(hSubKey, REGISTRY_SPECIFIC_TYPE, 0, REG_DWORD, (BYTE*)&trapCode, sizeof(trapCode)) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

	// add value REGISTRY_GENERIC_TYPE
	trapCode = 6;
    if (RegSetValueEx(hSubKey, REGISTRY_GENERIC_TYPE, 0, REG_DWORD, (BYTE*)&trapCode, sizeof(trapCode)) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

	// add value REGISTRY_DESCRIPTION
    if (RegSetValueEx(hSubKey, REGISTRY_DESCRIPTION, 0, REG_SZ, (BYTE*)description, strlen(description)+1) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

	// add value REGISTRY_PLUGIN
	trapCode = DEFAULT_PLUGIN;
    if (RegSetValueEx(hSubKey, REGISTRY_PLUGIN, 0, REG_DWORD, (BYTE*)&trapCode, sizeof(trapCode)) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

    RegCloseKey(hSubKey);

    return result;
}

void registryClose(HKEY hKey)
{
    RegCloseKey(hKey);
}
