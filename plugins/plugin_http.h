#ifndef __PLUGIN_HTTP_H__
#define __PLUGIN_HTTP_H__

#include <windows.h>
#include <stdint.h>
#include <ws2tcpip.h>

typedef struct _plugin_configuration
{
	char address[256];
	uint32_t port;
	char script[256];
} plugin_configuration;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* ws2tcpip BUG, not detecting Windows XP sevice pack compliante */
#ifndef freeaddrinfo
	void WSAAPI freeaddrinfo (struct addrinfo*);
#endif
#ifndef getaddrinfo
	int WSAAPI getaddrinfo (const char*,const char*,const struct addrinfo*, struct addrinfo**);
#endif
#ifndef getnameinfo
	int WSAAPI getnameinfo(const struct sockaddr*,socklen_t,char*,DWORD, char*,DWORD,int);
#endif

#endif // __PLUGIN_HTTP_H__

