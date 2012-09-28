#include <windows.h>

LONG loadActionList(HWND hDlg);
void freeActionList(HWND hDlg);
void refreshActionList(HWND hDlg);

BOOL loadAction(HWND hDlg, DWORD selected);
BOOL addAction(HWND hDlg, LPCTSTR oid);
BOOL deleteAction(HWND hDlg, DWORD selected);
BOOL saveAction(HWND hDlg, DWORD selected);
BOOL exportData(LPCTSTR filename);
void loadPlugins(HWND hDlg);
void configurePlugin(HWND hDlg);
