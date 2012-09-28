#include <windows.h>

LONG inline RegCountSubKeys(HKEY hKey, LPDWORD lpcSubKeys);
LONG RegCreateSubKey(HKEY hKey, DWORD dwOptions, REGSAM samDesired, HKEY *hSubKey);
#ifndef RegGetValue
LONG RegGetValue(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValue,
                        DWORD __reserved, LPDWORD pdwType, PVOID pvData, LPDWORD pcbData);
#define RRF_RT_ANY				NULL
#define RRF_RT_REG_BINARY		REG_BINARY
#define RRF_RT_REG_DWORD		REG_DWORD
#define RRF_RT_REG_EXPAND_SZ	REG_EXPAND_SZ
#define RRF_RT_REG_MULTI_SZ		REG_MULTI_SZ
#define RRF_RT_REG_NONE			REG_NONE
#define RRF_RT_REG_QWORD		REG_QWORD
#define RRF_RT_REG_SZ			REG_SZ
#endif
BOOL regExportPath(HKEY hRootKey, LPCTSTR regPath, LPCTSTR filename);

//RRF_RT_REG_SZ
