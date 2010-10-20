#include <windows.h>

LONG inline RegCountSubKeys(HKEY hKey, LPDWORD lpcSubKeys);
LONG RegCreateSubKey(HKEY hKey, DWORD dwOptions, REGSAM samDesired, HKEY *hSubKey);
#ifndef RegGetValue
LONG RegGetValue(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValue,
                        DWORD __reserved, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
#endif
BOOL regExportPath(HKEY hRootKey, LPCTSTR regPath, LPCTSTR filename);
