#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainDialogData.h"
#include "resource.h"
#include "config.h"
#include "addDialog.h"
#include "serviceController.h"
#include "mainDialog.h"

LONG inline RegCountSubKeys(HKEY hKey, LPDWORD lpcSubKeys)
{
    return RegQueryInfoKey(hKey, NULL, NULL, NULL, lpcSubKeys, NULL,  NULL, NULL, NULL, NULL, NULL, NULL);
}

LONG loadOidList(HWND hDlg)
{
    HKEY hKey;
    DWORD keyIndex, keyCount;
    char readBuffer[MAX_REGISTRY_LEN];
    DWORD readBufferSize;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return 0;
    }

    if (RegCountSubKeys(hKey, &keyCount) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    keyIndex = 0;
    readBufferSize = sizeof(readBuffer);
    while((RegEnumKeyEx(hKey, keyIndex, readBuffer, &readBufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
          && (keyIndex < keyCount))
    {
        SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)readBuffer);
        readBufferSize = sizeof(readBuffer); keyIndex++;
    }

    RegCloseKey(hKey);

    return keyIndex;
}

BOOL deleteAction(HWND hDlg, LPCTSTR oid)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegDeleteKey(hKey, oid) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;
}

BOOL addAction(LPCTSTR oid)
{
    HKEY hKey, hSubKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegCreateKeyEx(hKey, oid, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hSubKey, NULL) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    return TRUE;
}

BOOL loadAction(HWND hDlg, LPCTSTR oid)
{
    HKEY hKey;
    HKEY hSubKey;

    BYTE buffer[MAX_REGISTRY_LEN];
    DWORD bufferSize;

    DWORD numeric;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegOpenKeyEx(hKey, oid, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, "specificType", NULL, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
    {
        memcpy(&numeric, buffer, sizeof(numeric));
        snprintf((char *)buffer, sizeof(buffer), "%lu", numeric);

        SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessage(hDlg, ID_RADIO_GENERIC, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessage(hDlg, ID_RADIO_SPECIFIC, BM_SETCHECK, BST_CHECKED, 0);

        SendDlgItemMessage(hDlg, ID_EDIT_TRAP_CODE, WM_SETTEXT, 0, (LPARAM)buffer);
    }
    else
    {
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hSubKey, "genericType", NULL, NULL, buffer, &bufferSize) ==  ERROR_SUCCESS)
        {
            memcpy(&numeric, buffer, sizeof(numeric));
            snprintf((char *)buffer, sizeof(buffer), "%lu", numeric);

            SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_SETCHECK, BST_UNCHECKED, 0);
            SendDlgItemMessage(hDlg, ID_RADIO_GENERIC, BM_SETCHECK, BST_CHECKED, 0);
            SendDlgItemMessage(hDlg, ID_RADIO_SPECIFIC, BM_SETCHECK, BST_UNCHECKED, 0);

            SendDlgItemMessage(hDlg, ID_EDIT_TRAP_CODE, WM_SETTEXT, 0, (LPARAM)buffer);
        }
        else
        {
            SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_SETCHECK, BST_CHECKED, 0);
            SendDlgItemMessage(hDlg, ID_RADIO_GENERIC, BM_SETCHECK, BST_UNCHECKED, 0);
            SendDlgItemMessage(hDlg, ID_RADIO_SPECIFIC, BM_SETCHECK, BST_UNCHECKED, 0);

            SendDlgItemMessage(hDlg, ID_EDIT_TRAP_CODE, WM_SETTEXT, 0, (LPARAM)"");
            enableDlgItem(hDlg, ID_EDIT_TRAP_CODE, FALSE);
        }
    }

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, "run", NULL, NULL, buffer, &bufferSize) != ERROR_SUCCESS)
    {
            buffer[0] = '\0';
    }
    SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_SETTEXT, 0, (LPARAM)buffer);

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, "wkdir", NULL, NULL, buffer, &bufferSize) != ERROR_SUCCESS)
    {
            buffer[0] = '\0';
    }
    SendDlgItemMessage(hDlg, ID_EDIT_WKDIR, WM_SETTEXT, 0, (LPARAM)buffer);

    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    enableDlgItem(hDlg, ID_BUTTON_MODIFY, FALSE);

    return TRUE;
}

BOOL isNumber(LPBYTE buffer)
{
    // empty string not allowed
    if (*buffer == '\0')
    {
        return FALSE;
    }

    while(*buffer != '\0')
    {
        if (*buffer < '0' || *buffer > '9')
        {
            return FALSE;
        }
        buffer++;
    }
    return TRUE;
}

BOOL saveAction(HWND hDlg, LPCTSTR oid)
{
    HKEY hKey;
    HKEY hSubKey;

    BYTE buffer[MAX_REGISTRY_LEN];
    DWORD bufferSize;

    DWORD numeric;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegOpenKeyEx(hKey, oid, 0, KEY_READ|KEY_WRITE, &hSubKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        // delete values speficicType and genericType
        RegDeleteValue(hSubKey, "specificType");
        RegDeleteValue(hSubKey, "genericType");
    }
    else
    {
        SendDlgItemMessage(hDlg, ID_EDIT_TRAP_CODE, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
        if (isNumber(buffer) != TRUE)
        {
            MessageBox(hDlg, "Invalid trap code!", "Error", MB_ICONERROR|MB_OK);

            // delete values speficicType and genericType
            RegDeleteValue(hSubKey, "specificType");
            RegDeleteValue(hSubKey, "genericType");
        }
        else
        {
            sscanf((char *)buffer, "%lu", &numeric);
            memcpy(buffer, &numeric, sizeof(numeric));

            if (SendDlgItemMessage(hDlg, ID_RADIO_SPECIFIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                // wirte value specificType
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, "specificType", 0, REG_DWORD, buffer, bufferSize);

                // write value 6 as genericType
                numeric = 6;
                memcpy(buffer, &numeric, sizeof(numeric));
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, "genericType", 0, REG_DWORD, buffer, bufferSize);
            }
            else if (SendDlgItemMessage(hDlg, ID_RADIO_GENERIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                // write value genericType
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, "genericType", 0, REG_DWORD, buffer, bufferSize);

                // delete value specificType
                RegDeleteValue(hSubKey, "specificType");
            }
        }
    }

    SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
    bufferSize = strlen((char *)buffer)+1;
    if (bufferSize > 1)
    {
        RegSetValueEx(hSubKey, "run", 0, REG_SZ, buffer, bufferSize);
    }
    else
    {
        RegDeleteValue(hSubKey, "run");
    }

    SendDlgItemMessage(hDlg, ID_EDIT_WKDIR, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
    bufferSize = strlen((char *)buffer)+1;
    if (bufferSize > 1)
    {
        RegSetValueEx(hSubKey, "wkdir", 0, REG_SZ, buffer, bufferSize);
    }
    else
    {
        RegDeleteValue(hSubKey, "wkdir");
    }

    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    return TRUE;
}

BOOL exportData(LPCTSTR filename)
{
    char buffer[MAX_PATH+512];

    snprintf(buffer, sizeof(buffer), "/e \"%s\" HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\snmpTrapHandler\\dispatcher", filename);
    ShellExecute(NULL, "open", "regedit.exe", buffer, NULL, SW_SHOW);

    return TRUE;
}
