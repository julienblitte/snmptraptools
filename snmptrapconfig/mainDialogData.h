#include <windows.h>

LONG loadOidList(HWND hDlg);

BOOL loadAction(HWND hDlg, LPCTSTR oid);
BOOL addAction(LPCTSTR oid);
BOOL deleteAction(HWND hDlg, LPCTSTR oid);
BOOL saveAction(HWND hDlg, LPCTSTR oid);
BOOL exportData(LPCTSTR filename);
