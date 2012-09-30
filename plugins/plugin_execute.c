#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Shlobj.h>

#include "plugin_execute.h"
#include "plugin_execute_resources.h"
#include "..\core\trapSnmp.h"
#include "..\core\plugin_common.h"

static const char plugin_name[] = "Execute";
static const unsigned int plugin_uid = STR2UID('E','X','E','C');;

static plugin_configuration config;

static HINSTANCE hinstance;

DLL_EXPORT const char *GETNAME()
{
	return plugin_name;
}

DLL_EXPORT void LOADCONFIG(const void *data, const unsigned int data_size)
{
	if (data_size == sizeof(config))
	{
		memcpy((void *)&config, data, data_size);
	}

	return;
}

DLL_EXPORT void *EDITCONFIG(void *data, unsigned int *data_size)
{
	// request for new default values
	if (data == NULL || *data_size != sizeof(config))
	{
		strncpy(config.run, "", sizeof(config.run));
		strncpy(config.wkdir, "", sizeof(config.wkdir));
		*data_size = sizeof(config);
	}
	else
	{
		LoadConfig(data, *data_size);
		DialogBox(hinstance, MAKEINTRESOURCE(ID_DIALOG_CONFIG), NULL, (DLGPROC)dlgAddMessageHandler);
	}

	return &config;
}

DLL_EXPORT void RUN(snmpTrap *trap)
{
	static char executeContent[512];
	HINSTANCE hinst;

    snprintf(executeContent, sizeof(executeContent), "\"%s\" \"%s\" %u %u", trap->agent, trap->enterprise, trap->generic_type, trap->specific_type);
    hinst = ShellExecute(NULL, "open", config.run, executeContent, config.wkdir, SW_SHOW);

	if ((int)hinst <= 32)
	{
		snprintf(executeContent, sizeof(executeContent), "Error %d executing \"%s\".", (int)hinst, config.run);
		MessageBox(0, executeContent, plugin_name, MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
	}
}

DLL_EXPORT unsigned int GETUID()
{
	return plugin_uid;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
			hinstance = hinstDLL;
            break;
		case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#ifndef rnindex
char *rnindex(const char *s, char c, size_t max)
{
	char *result;
	int i;

	result = NULL;
	i = 0;
	while(i < max)
	{
		if (s[i] == '\0')	break;
		if (s[i] == c)		result = (char *)&s[i];
		i++;
	}
	return result;
}
#endif

BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME    openFile;
	BROWSEINFO		dir;
	LPITEMIDLIST	pidl;
	char browse_file[MAX_PATH];
	char *backslash;

    switch(uMsg)
    {
        case WM_INITDIALOG:
			SetDlgItemText(hDlg, ID_EDIT_RUN, config.run);
			SetDlgItemText(hDlg, ID_EDIT_WORK_DIR, config.wkdir);
            return TRUE;

        case WM_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;

        case WM_COMMAND:
			switch(wParam)
			{
				case ON_CLICK|ID_BUTTON_OK:
					GetDlgItemText(hDlg, ID_EDIT_RUN, config.run, sizeof(config.run));
					GetDlgItemText(hDlg, ID_EDIT_WORK_DIR, config.wkdir, sizeof(config.wkdir));
					EndDialog(hDlg, 0);
					return TRUE;

				case ON_CLICK|ID_BUTTON_CANCEL:
					EndDialog(hDlg, 0);
					return TRUE;

				case ON_CLICK|ID_BROWSE_RUN:
					ZeroMemory(&openFile, sizeof(OPENFILENAME));
					strncpy(browse_file, "", sizeof(browse_file));

					openFile.lStructSize = sizeof(OPENFILENAME);
					openFile.hwndOwner = hDlg;
					openFile.hInstance = hinstance;
					openFile.lpstrFilter = "Executables\0*.exe;*.com;*.class;*.bat;*.cmd;*.pl;*.php;*.vbs;*.js;*.asp;\0All files\0*.*\0\0";
					openFile.lpstrFile = browse_file;
					openFile.nMaxFile = sizeof(browse_file);
					openFile.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

					if (GetOpenFileName(&openFile))
					{
						SendDlgItemMessage(hDlg, ID_EDIT_RUN, WM_SETTEXT, 0, (LPARAM)browse_file);
						backslash = rnindex(browse_file, '\\', sizeof(browse_file));
						if (backslash != NULL)
						{
							*backslash = '\0';
							SendDlgItemMessage(hDlg, ID_EDIT_WORK_DIR, WM_SETTEXT, 0, (LPARAM)browse_file);
						}
					}
					return TRUE;

				case ON_CLICK|ID_BROWSE_WORK_DIR:
					dir.hwndOwner = hDlg;
					dir.pszDisplayName = browse_file;
					dir.lpszTitle = "Select directory";
					dir.ulFlags = BIF_RETURNONLYFSDIRS;

					ZeroMemory(&dir, sizeof(BROWSEINFO));
					strncpy(browse_file, "", sizeof(browse_file));

					dir.hwndOwner = hDlg;
					dir.pszDisplayName = browse_file;
					dir.lpszTitle = "Select directory";
					dir.ulFlags = BIF_RETURNONLYFSDIRS;

					pidl = SHBrowseForFolder(&dir);
					if (pidl != NULL)
					{
						SHGetPathFromIDList(pidl, browse_file);
						SendDlgItemMessage(hDlg, ID_EDIT_WORK_DIR, WM_SETTEXT, 0, (LPARAM)browse_file);
					}
					return TRUE;
			}
			return FALSE;
    }
    return FALSE;
}
