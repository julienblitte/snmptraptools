#ifndef MAINDIALOG_H_INCLUDED
#define MAINDIALOG_H_INCLUDED

#include <windows.h>

extern HINSTANCE hInst;

BOOL dlgMainEventHandler(HWND hDlg, WPARAM wParam);
BOOL CALLBACK dlgMainMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL enableDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable);
BOOL enableActionModification(HWND hDlg, BOOL state);
BOOL updateServiceState(HWND hDlg);
BOOL proposeServiceUpdate(HWND hDlg);

#endif // MAINDIALOG_H_INCLUDED
