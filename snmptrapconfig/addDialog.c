#include <windows.h>
#include <regex.h>
#include "resource.h"
#include "mainDialogData.h"
#include "../snmptraptool_config.h"
#include "../libtrapsnmp/trapSnmp.h"

HWND hMainDialog = NULL;

BOOL dlgAddEventHandler(HWND hDlg, WPARAM wParam)
{
    char buffer[MAX_OID_LEN];

    switch(wParam)
    {
        case ON_CLICK|ID_BUTTON_ADD_OID:
            SendDlgItemMessage(hDlg, ID_EDIT_OID, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);

            if (oidIsValid(buffer) == FALSE)
            {
                MessageBox(hDlg, "You must enter a valid OID!\nOid have to start by a digit, not a dot.", "Error", MB_ICONERROR|MB_OK);
            }
            else
            {

                if (addAction((LPCTSTR)buffer) == TRUE)
                {
                    SendDlgItemMessage(hMainDialog, ID_LISTBOX_OID, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)buffer);
                }

                EndDialog(hDlg, 0);
            }
            return TRUE;

        case ON_CLICK|ID_BUTTON_CANCEL_OID:
            EndDialog(hDlg, 0);
            return TRUE;
    }
    return FALSE;
}


BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;

        case WM_COMMAND:
            return dlgAddEventHandler(hDlg, wParam);
    }

    return FALSE;
}

void showAddDlg(HINSTANCE hInst, HWND hParent)
{
    hMainDialog = hParent;

    DialogBox(hInst, MAKEINTRESOURCE(ID_DIALOG_ADD), hParent, (DLGPROC)dlgAddMessageHandler);
}

