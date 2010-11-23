#include <windows.h>
#include "registry.h"
#include "../snmptraptools_config.h"

char registryWkDir[MAX_PATH];
DWORD registryWkDir_size;

HKEY registryOpen()
{
	HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return NULL;
    }

    registryWkDir_size = sizeof(registryWkDir);
    if (registryServiceDirectory(registryWkDir, &registryWkDir_size) != TRUE)
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

	// add value REGISTRY_RUN
    if (RegSetValueEx(hSubKey, REGISTRY_RUN, 0, REG_SZ, (BYTE*)DEFAULT_ACTION, strlen(DEFAULT_ACTION)+1) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

	// add value REGISTRY_WKDIR
    if (RegSetValueEx(hSubKey, REGISTRY_WKDIR, 0, REG_SZ, (BYTE*)registryWkDir, registryWkDir_size) != ERROR_SUCCESS)
    {
		result = FALSE;
    }

    RegCloseKey(hSubKey);

    // client side: refreshActionList(hDlg);

    return result;
}

void registryClose(HKEY hKey)
{
    RegCloseKey(hKey);
}

BOOL registryServiceDirectory(char *path, DWORD *path_size)
{
	char *filename;
	HKEY serviceKey;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_SERVICE_PATH, 0, KEY_READ, &serviceKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if (RegQueryValueEx(serviceKey, REGISTRY_SERVICE_EXECUTABLE, NULL, NULL, (BYTE*)path, path_size) != ERROR_SUCCESS)
	{
		RegCloseKey(serviceKey);
		return FALSE;
	}

	// directory and filename split
	filename = strrchr(path, '\\');
	if (filename != NULL)
	{
		*filename = '\0';
		filename++;
	}

	RegCloseKey(serviceKey);

	return TRUE;
}

