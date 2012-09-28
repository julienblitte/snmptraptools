#ifndef __PLUGIN_SYSLOG_H__
#define __PLUGIN_SYSLOG_H__

#include <windows.h>

typedef struct _plugin_configuration
{
	char target[16];
	char application[32];
} plugin_configuration;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define SYSLOG_FACILITY_USER_LEVEL	1
#define SYSLOG_SEVERITY_NOTICE		5

#define SYSLOG_SERVER_PORT	514

#endif // __PLUGIN_SYSLOG_H__

