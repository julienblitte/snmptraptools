#include <windows.h>
#include <stdio.h>

LONG inline RegCountSubKeys(HKEY hKey, LPDWORD lpcSubKeys)
{
    return RegQueryInfoKey(hKey, NULL, NULL, NULL, lpcSubKeys, NULL,  NULL, NULL, NULL, NULL, NULL, NULL);
}

LONG RegCreateSubKey(HKEY hKey, DWORD dwOptions, REGSAM samDesired, HKEY *hSubKey)
{
    DWORD disposition;
    char subKeyName[32];
    DWORD counter;
    LONG error;

    counter = 0;
    do
    {
        snprintf(subKeyName, sizeof(subKeyName), "%u", (unsigned)counter);
        error = RegCreateKeyEx(hKey, subKeyName, 0, NULL, dwOptions, samDesired, NULL, hSubKey, &disposition);

        if (error != ERROR_SUCCESS)
        {
            return error;
        }

        counter++;
    } while (disposition != REG_CREATED_NEW_KEY);

    return ERROR_SUCCESS;
}

// this function only exists since Windows Vista
#ifndef RegGetValue
LONG RegGetValue(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValue,
                        DWORD __reserved, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData)
{
    HKEY hSubKey;
    LONG errorCode;

    errorCode = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hSubKey);
    if (errorCode != ERROR_SUCCESS)
    {
        return errorCode;
    }

    errorCode = RegQueryValueEx(hSubKey, lpValue, NULL, pdwType, pvData, pcbData);
    if (errorCode != ERROR_SUCCESS)
    {
        RegCloseKey(hSubKey);
        return errorCode;
    }

    RegCloseKey(hSubKey);

    return ERROR_SUCCESS;
}
#endif

BOOL regExportPath(HKEY hRootKey, LPCTSTR regPath, LPCTSTR filename)
{
    char param[MAX_PATH];
    HINSTANCE regedit;
    char *rootKeyName;

    switch ((unsigned long)hRootKey)
    {
        case (unsigned long)HKEY_CLASSES_ROOT:
            rootKeyName = "HKEY_CLASSES_ROOT";
            break;
        case (unsigned long)HKEY_CURRENT_CONFIG:
            rootKeyName = "HKEY_CURRENT_CONFIG";
            break;
        case (unsigned long)HKEY_CURRENT_USER:
        default:
            rootKeyName = "HKEY_CURRENT_USER";
            break;
        case (unsigned long)HKEY_LOCAL_MACHINE:
            rootKeyName = "HKEY_LOCAL_MACHINE";
            break;
        case (unsigned long)HKEY_USERS:
            rootKeyName = "HKEY_USERS";
            break;
    }


    snprintf(param, sizeof(param), "/e \"%s\" \"%s\\%s\"", filename, rootKeyName, regPath);

    regedit = ShellExecute(NULL, "open", "regedit.exe", param, NULL, SW_SHOW);

    return ((int)regedit < 32);
}
