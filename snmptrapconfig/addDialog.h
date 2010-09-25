#ifndef ADDDIALOG_H_INCLUDED
#define ADDDIALOG_H_INCLUDED

#include <windows.h>

BOOL dlgAddEventHandler(HWND hDlg, WPARAM wParam);
BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void showAddDlg(HINSTANCE hInst, HWND hParent);

#endif // ADDDIALOG_H_INCLUDED
