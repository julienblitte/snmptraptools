#include <windows.h>

HKEY registryOpen();
BOOL registryAddOid(HKEY hKey, LPCTSTR oid, DWORD trapCode, LPCTSTR description);
void registryClose(HKEY hKey);
