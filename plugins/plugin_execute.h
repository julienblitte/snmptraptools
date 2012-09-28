#ifndef __PLUGIN_EXECUTE_H__
#define __PLUGIN_EXECUTE_H__

#include <windows.h>

typedef struct _plugin_configuration
{
	char run[512];
	char wkdir[512];
} plugin_configuration;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // __PLUGIN_EXECUTE_H__

