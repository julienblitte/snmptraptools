#ifndef PLUGIN_SMTP_H_INCLUDED
#define PLUGIN_SMTP_H_INCLUDED

#include <ws2tcpip.h>

typedef struct _plugin_configuration
{
	char server[128];
	char from[128];
	char to[128];
	bool authentication;
	char login[64];
	char password[64];
} plugin_configuration;


BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);


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

#endif // PLUGIN_SMTP_H_INCLUDED
