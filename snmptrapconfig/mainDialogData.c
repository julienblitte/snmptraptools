#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mainDialogData.h"
#include "resource.h"
#include "..\core\snmptraptools_config.h"
#include "addDialog.h"
#include "serviceController.h"
#include "mainDialog.h"
#include "../libregistry/registry.h"

char **actionList = NULL;

LONG loadActionList(HWND hDlg)
{
    HKEY hKey, hSubKey;
    DWORD keyIndex, keyCount;
    char subKeyName[MAX_REGISTRY_LEN];
    DWORD subKeyNameSize;
    char oid[MAX_OID_LEN];
    DWORD oidSize;

    // open registry
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return 0;
    }

    // get number of subkeys
    if (RegCountSubKeys(hKey, &keyCount) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 0;
    }

    actionList = calloc(keyCount, sizeof(char *));
    if (actionList == NULL)
    {
        return 0;
    }

    // for each subkey
    keyIndex = 0;
    subKeyNameSize = sizeof(subKeyName);
    while((RegEnumKeyEx(hKey, keyIndex, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
          && (keyIndex < keyCount))
    {
        // TODO: read oid value
        oidSize = sizeof(oid);
        if (RegGetValue(hKey, subKeyName, REGISTRY_OID, 0, NULL, oid, &oidSize) != ERROR_SUCCESS)
        {
            // Oops, problem detected in registry
            // maybe an old version of program was installed before

            // fill missing oid with subKeyName (old version's configuration format)
            strncpy(oid, subKeyName, sizeof(oid));

            // try to update configuration's format in registry
            if (RegCreateKeyEx(hKey, subKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hSubKey, NULL) == ERROR_SUCCESS)
            {
                RegSetValueEx(hSubKey, REGISTRY_OID, 0, REG_SZ, (BYTE*)oid, strlen(oid)+1);
                RegCloseKey(hSubKey);
            }
        }
        SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)oid);
        actionList[keyIndex] = strdup(subKeyName);
        keyIndex++; subKeyNameSize = sizeof(subKeyName);
    }

    RegCloseKey(hKey);

    return keyIndex;
}

void freeActionList(HWND hDlg)
{
    SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_RESETCONTENT, 0, 0);
    free(actionList);
}

void refreshActionList(HWND hDlg)
{
    freeActionList(hDlg);
    loadActionList(hDlg);
}

BOOL deleteAction(HWND hDlg, DWORD selected)
{
    HKEY hKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegDeleteKey(hKey, actionList[selected]) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);

    refreshActionList(hDlg);

    return TRUE;
}

BOOL addAction(HWND hDlg, LPCTSTR oid)
{
    HKEY hKey, hSubKey;

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_CONFIG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    // add a new subkey
    if (RegCreateSubKey(hKey, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, &hSubKey) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    // add value REGISTRY_OID in this key
    if (RegSetValueEx(hSubKey, REGISTRY_OID, 0, REG_SZ, (BYTE*)oid, strlen(oid)+1) != ERROR_SUCCESS)
    {
        RegCloseKey(hSubKey);
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    refreshActionList(hDlg);

    return TRUE;
}

BOOL loadAction(HWND hDlg, DWORD selected)
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

    printf("%u: %s\n", (unsigned int)selected, actionList[selected]);

    // TODO: retrive oid from selected index...
    if (RegOpenKeyEx(hKey, actionList[selected], 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, REGISTRY_SPECIFIC_TYPE, NULL, NULL, buffer, &bufferSize) == ERROR_SUCCESS)
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
        if (RegQueryValueEx(hSubKey, REGISTRY_GENERIC_TYPE, NULL, NULL, buffer, &bufferSize) ==  ERROR_SUCCESS)
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
    if (RegQueryValueEx(hSubKey, REGISTRY_DESCRIPTION, NULL, NULL, buffer, &bufferSize) != ERROR_SUCCESS)
    {
            buffer[0] = '\0';
    }
    SendDlgItemMessage(hDlg, ID_EDIT_DESCRIPTION, WM_SETTEXT, 0, (LPARAM)buffer);

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, REGISTRY_RUN, NULL, NULL, buffer, &bufferSize) != ERROR_SUCCESS)
    {
            buffer[0] = '\0';
    }
    SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_SETTEXT, 0, (LPARAM)buffer);

    bufferSize = sizeof(buffer);
    if (RegQueryValueEx(hSubKey, REGISTRY_WKDIR, NULL, NULL, buffer, &bufferSize) != ERROR_SUCCESS)
    {
            buffer[0] = '\0';
    }
    SendDlgItemMessage(hDlg, ID_EDIT_WORK_DIR, WM_SETTEXT, 0, (LPARAM)buffer);

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

BOOL saveAction(HWND hDlg, DWORD selected)
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

    if (RegOpenKeyEx(hKey, actionList[selected], 0, KEY_READ|KEY_WRITE, &hSubKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        // delete values speficicType and genericType
        RegDeleteValue(hSubKey, REGISTRY_SPECIFIC_TYPE);
        RegDeleteValue(hSubKey, REGISTRY_GENERIC_TYPE);
    }
    else
    {
        SendDlgItemMessage(hDlg, ID_EDIT_TRAP_CODE, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
        if (isNumber(buffer) != TRUE)
        {
            MessageBox(hDlg, "Invalid trap code!", "Error", MB_ICONERROR|MB_OK);

            // delete values speficicType and genericType
            RegDeleteValue(hSubKey, REGISTRY_SPECIFIC_TYPE);
            RegDeleteValue(hSubKey, REGISTRY_GENERIC_TYPE);
        }
        else
        {
            sscanf((char *)buffer, "%lu", &numeric);
            memcpy(buffer, &numeric, sizeof(numeric));

            if (SendDlgItemMessage(hDlg, ID_RADIO_SPECIFIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                // wirte value specificType
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, REGISTRY_SPECIFIC_TYPE, 0, REG_DWORD, buffer, bufferSize);

                // write value 6 as genericType
                numeric = 6;
                memcpy(buffer, &numeric, sizeof(numeric));
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, REGISTRY_GENERIC_TYPE, 0, REG_DWORD, buffer, bufferSize);
            }
            else if (SendDlgItemMessage(hDlg, ID_RADIO_GENERIC, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                // write value genericType
                bufferSize = sizeof(numeric);
                RegSetValueEx(hSubKey, REGISTRY_GENERIC_TYPE, 0, REG_DWORD, buffer, bufferSize);

                // delete value specificType
                RegDeleteValue(hSubKey, REGISTRY_SPECIFIC_TYPE);
            }
        }
    }

    SendDlgItemMessage(hDlg, ID_EDIT_DESCRIPTION, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
    bufferSize = strlen((char *)buffer)+1;
    if (bufferSize > 1)
    {
        RegSetValueEx(hSubKey, REGISTRY_DESCRIPTION, 0, REG_SZ, buffer, bufferSize);
    }
    else
    {
        RegDeleteValue(hSubKey, REGISTRY_DESCRIPTION);
    }

    SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
    bufferSize = strlen((char *)buffer)+1;
    if (bufferSize > 1)
    {
        RegSetValueEx(hSubKey, REGISTRY_RUN, 0, REG_SZ, buffer, bufferSize);
    }
    else
    {
        RegDeleteValue(hSubKey, REGISTRY_RUN);
    }

    SendDlgItemMessage(hDlg, ID_EDIT_WORK_DIR, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
    bufferSize = strlen((char *)buffer)+1;
    if (bufferSize > 1)
    {
        RegSetValueEx(hSubKey, REGISTRY_WKDIR, 0, REG_SZ, buffer, bufferSize);
    }
    else
    {
        RegDeleteValue(hSubKey, REGISTRY_WKDIR);
    }

    RegCloseKey(hSubKey);
    RegCloseKey(hKey);

    return TRUE;
}

