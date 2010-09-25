#include <windows.h>
#include <stdio.h>
#include <Shlobj.h>
#include "mainDialogData.h"
#include "serviceController.h"
#include "resource.h"
#include "config.h"
#include "mainDialog.h"
#include "addDialog.h"

BOOL proposeServiceUpdate(HWND hDlg)
{
    if (getServiceState() == SERVICE_RUNNING)
    {
        enableDlgItem(hDlg, ID_BUTTON_UPDATE, TRUE);
        return TRUE;
    }

    return FALSE;
}

BOOL updateServiceState(HWND hDlg)
{
    DWORD serviceSate;

    serviceSate = getServiceState();
    switch (serviceSate)
    {
    case SERVICE_RUNNING:
    case SERVICE_PAUSED:
        enableDlgItem(hDlg, ID_BUTTON_START, FALSE);
        enableDlgItem(hDlg, ID_BUTTON_STOP, TRUE);
        break;
    case SERVICE_STOPPED:
        enableDlgItem(hDlg, ID_BUTTON_START, TRUE);
        enableDlgItem(hDlg, ID_BUTTON_STOP, FALSE);
        break;
    }

    return TRUE;
}

BOOL enableDlgItem(HWND hDlg, int nIDDlgItem, BOOL bEnable)
{
    HWND hwnd;
    hwnd = GetDlgItem(hDlg, nIDDlgItem);
    if (hwnd != NULL)
    {
        return EnableWindow(hwnd, bEnable);
    }
    return !bEnable;
}

BOOL enableActionModification(HWND hDlg, BOOL state)
{
    enableDlgItem(hDlg, ID_GROUPBOX_TRAPTYPE, state);
    enableDlgItem(hDlg, ID_RADIO_ANY, state);
    enableDlgItem(hDlg, ID_RADIO_GENERIC, state);
    enableDlgItem(hDlg, ID_RADIO_SPECIFIC, state);

    if (SendDlgItemMessage(hDlg, ID_RADIO_ANY, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        enableDlgItem(hDlg, ID_EDIT_TRAP_CODE, FALSE);
        enableDlgItem(hDlg, ID_CAPTION_TRAP_CODE, FALSE);
    }
    else
    {
        enableDlgItem(hDlg, ID_EDIT_TRAP_CODE, state);
        enableDlgItem(hDlg, ID_CAPTION_TRAP_CODE, state);
    }

    enableDlgItem(hDlg, ID_EDIT_RUN, state);
    enableDlgItem(hDlg, ID_BROWSE_RUN, state);

    enableDlgItem(hDlg, ID_EDIT_WKDIR, state);
    enableDlgItem(hDlg, ID_BROWSE_WKDIR, state);

    return state;
}

BOOL GetOpenDirectory(HWND hDlg, char *buffer)
{
	BROWSEINFO dir;
	LPITEMIDLIST pidl;

	dir.hwndOwner = hDlg;
	dir.pszDisplayName = buffer;
	dir.lpszTitle = "Select directory";
	dir.ulFlags = BIF_RETURNONLYFSDIRS;

	ZeroMemory(&dir, sizeof(BROWSEINFO));
	buffer[0] = '\0';

	dir.hwndOwner = hDlg;
	dir.pszDisplayName = buffer;
	dir.lpszTitle = "Select directory";
	dir.ulFlags = BIF_RETURNONLYFSDIRS;

	pidl = SHBrowseForFolder(&dir);
	if (pidl != NULL)
	{
		SHGetPathFromIDList(pidl, buffer);
		return TRUE;
	}
	return FALSE;
}

BOOL dlgMainEventHandler(HWND hDlg, WPARAM wParam)
{
    char oid[MAX_OID_LEN];
    int selected;

    OPENFILENAME    openFile;

    char buffer[MAX_PATH];

    switch(wParam)
    {
        case ON_CLICK|ID_BUTTON_START:
            setServiceState(SERVICE_CONTROL_START);
            updateServiceState(hDlg);
            return TRUE;
        case ON_CLICK|ID_BUTTON_STOP:
            setServiceState(SERVICE_CONTROL_STOP);
            updateServiceState(hDlg);
            return TRUE;
        case ON_CLICK|ID_BUTTON_ADD:
            showAddDlg(hInst, hDlg);
            return TRUE;
        case ON_CLICK|ID_BUTTON_UPDATE:
            setServiceState(SERVICE_CONTROL_PARAMCHANGE);
            updateServiceState(hDlg);
            enableDlgItem(hDlg, ID_BUTTON_UPDATE, FALSE);
            return TRUE;
        case ON_CLICK|ID_BUTTON_REMOVE:
            selected = (int)SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETCURSEL, 0, 0);
            if (selected != LB_ERR)
            {
                SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETTEXT, (WPARAM)selected, (LPARAM)oid);
                if (MessageBox(hDlg, "Do you really want to remove this action?", oid, MB_ICONQUESTION|MB_YESNO) == IDYES)
                {
                    if (deleteAction(hDlg, oid) == TRUE)
                    {
                        SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_DELETESTRING, (WPARAM)selected, 0);
                        enableActionModification(hDlg, FALSE);
                        proposeServiceUpdate(hDlg);
                    }
                }
            }
            return TRUE;
        case ON_CLICK|ID_BUTTON_MODIFY:
            selected = (int)SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETCURSEL, 0, 0);
            if (selected != LB_ERR)
            {
                SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETTEXT, (WPARAM)selected, (LPARAM)oid);
                if (saveAction(hDlg, oid) == TRUE)
                {
                    loadAction(hDlg, oid);
                    proposeServiceUpdate(hDlg);
                }
            }
            return TRUE;
        case ON_CHANGE|ID_EDIT_TRAP_CODE:
            enableDlgItem(hDlg, ID_BUTTON_MODIFY, TRUE);
            return TRUE;
        case ON_CHANGE|ID_EDIT_RUN:
            enableDlgItem(hDlg, ID_BUTTON_MODIFY, TRUE);
            return TRUE;
        case ON_CHANGE|ID_EDIT_WKDIR:
            enableDlgItem(hDlg, ID_BUTTON_MODIFY, TRUE);
            return TRUE;
        case ON_CLICK|ID_RADIO_GENERIC:
        case ON_CLICK|ID_RADIO_SPECIFIC:
            enableDlgItem(hDlg, ID_EDIT_TRAP_CODE, TRUE);
            enableDlgItem(hDlg, ID_CAPTION_TRAP_CODE, TRUE);
            enableDlgItem(hDlg, ID_BUTTON_MODIFY, TRUE);
            return TRUE;
        case ON_CLICK|ID_RADIO_ANY:
            enableDlgItem(hDlg, ID_EDIT_TRAP_CODE, FALSE);
            enableDlgItem(hDlg, ID_CAPTION_TRAP_CODE, TRUE);
            enableDlgItem(hDlg, ID_BUTTON_MODIFY, TRUE);
            return TRUE;
        case ON_ITEM_SELECT|ID_LISTBOX_OID:
            enableActionModification(hDlg, FALSE);

            selected = (int)SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETCURSEL, 0, 0);
            if (selected != LB_ERR)
            {
                enableDlgItem(hDlg, ID_BUTTON_REMOVE, TRUE);

                SendDlgItemMessage(hDlg, ID_LISTBOX_OID, LB_GETTEXT, (WPARAM)selected, (LPARAM)oid);
                loadAction(hDlg, oid);
                enableActionModification(hDlg, TRUE);
            }
            else
            {
                enableDlgItem(hDlg, ID_BUTTON_REMOVE, FALSE);
            }
            return TRUE;
        case ON_CLICK|ID_BROWSE_RUN:
            ZeroMemory(&openFile, sizeof(OPENFILENAME));
            buffer[0] = '\0';

            openFile.lStructSize = sizeof(OPENFILENAME);
            openFile.hwndOwner = hDlg;
            openFile.hInstance = hInst;
            openFile.lpstrFilter = "Executables\0*.exe;*.com;*.class;*.bat;*.cmd;*.pl;*.php;*.vbs;*.js;*.asp;\0All files\0*.*\0\0";
            openFile.lpstrFile = buffer;
            openFile.nMaxFile = sizeof(buffer);
            openFile.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

            if (GetOpenFileName(&openFile))
            {
                SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_SETTEXT, 0, (LPARAM)buffer);
            }
            return TRUE;

        case ON_CLICK|ID_BROWSE_WKDIR:
            if (GetOpenDirectory(hDlg, buffer) == TRUE)
            {
                SendDlgItemMessage(hDlg, ID_EDIT_WKDIR, WM_SETTEXT, 0, (LPARAM)buffer);
            }
            return TRUE;
        case ON_CLICK|ID_BUTTON_EXPORT:
            ZeroMemory(&openFile, sizeof(OPENFILENAME));
            buffer[0] = '\0';

            openFile.lStructSize = sizeof(OPENFILENAME);
            openFile.hwndOwner = hDlg;
            openFile.hInstance = hInst;
            openFile.lpstrFilter = "registry files (*.reg)\0*.reg\0\0";
            openFile.lpstrFile = buffer;
            openFile.nMaxFile = sizeof(buffer);
            openFile.lpstrDefExt = "reg";
            openFile.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&openFile))
            {
                exportData(openFile.lpstrFile);
            }
            return TRUE;

        default:
            printf("%04x: %04x\n", LOWORD(wParam), HIWORD(wParam));
            break;
    }

    return FALSE;
}

BOOL CALLBACK dlgMainMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            openService();

            updateServiceState(hDlg);
            SetTimer(hDlg, 1, 500, NULL);

            loadOidList(hDlg);
            enableActionModification(hDlg, FALSE);
            enableDlgItem(hDlg, ID_BUTTON_UPDATE, FALSE);

            return TRUE;

        case WM_TIMER:
            updateServiceState(hDlg);
            return TRUE;

        case WM_CLOSE:
            closeService();
            EndDialog(hDlg, 0);
            return TRUE;

        case WM_COMMAND:
            return dlgMainEventHandler(hDlg, wParam);
    }

    return FALSE;
}


