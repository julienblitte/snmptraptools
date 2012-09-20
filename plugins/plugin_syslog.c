#include "plugin_syslog.h"
#include "..\core\trapSnmp.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <windows.h>
#include <winsock2.h>

static const char plugin_name[] = PLUGIN_NAME;
static unsigned int plugin_uid = PLUGIN_UID;

static plugin_configuration config;

const char syslog_month[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int syslog_compile_message(uint8_t facility, uint8_t severity, char *hostname, char *message, char *buffer, size_t buffer_size)
{
	uint8_t priority;
	int dom, h, m, s;
	const char *month;
	SYSTEMTIME time;

	priority = (facility<<3)+severity;

	GetLocalTime(&time);
	dom = time.wDay;
	h = time.wHour;
	m = time.wMinute;
	s = time.wSecond;
	month = syslog_month[time.wMonth];

	return snprintf(buffer, buffer_size, "<%u>%s %2d %02d:%02d:%02d %s %s",
						priority, month, dom, h, m, s, hostname, message);
}

int syslog_send_message(const char *message, const char *destination)
{
	struct sockaddr_in dest;
	SOCKET s;

	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(destination);
	dest.sin_port = htons(SYSLOG_SERVER_PORT);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	return sendto(s, message, strlen(message), 0, (struct sockaddr*)&dest, sizeof(dest));
}



DLL_EXPORT const char *GetName()
{
	return plugin_name;
}

DLL_EXPORT void LoadConfig(void *data, unsigned int data_size)
{
	return;
}

DLL_EXPORT void *EditConfig(void *data, unsigned int *data_size)
{
	// request for new default values
	if (data == NULL)
	{
		strncpy(config.application, "snmptraptools", sizeof(config.application));
		strncpy(config.target, "127.0.0.1", sizeof(config.target));
		data_size = sizeof(config);

		return &config;
	}
	else
	{
		MessageBoxA(0, "Parameter change no yet implemented for this plugin.\nIn the future, you should be able to change SYSLOG_DEST_APPLICATION and SYSLOG_DEST_IP parameters.", plugin_name, MB_OK | MB_ICONINFORMATION);
		// TODO: display windows to configure this both parameters, update local config and return it
	}

	return NULL;
}

DLL_EXPORT void Run(snmpTrap *trap)
{
	static char syslogContent[512];
	static char sendBuffer[1024];
	int net;

	snprintf(syslogContent, sizeof(syslogContent), "%s: %s %d \"%s\"", config.application, trap->enterprise,
             snmptrap_code(trap), snmptrap_description(trap));

	syslog_compile_message(SYSLOG_FACILITY_USER_LEVEL, SYSLOG_SEVERITY_NOTICE, trap->agent, syslogContent, sendBuffer, sizeof(sendBuffer));
	net = syslog_send_message(sendBuffer, conf.target);

	if (net < 0)
	{
		net = GetLastError();
		snprintf(sendBuffer, sizeof(syslogContent), "Error sending message to %s.\n\nSystem report error %d.", conf.target, net);
		MessageBox(0, sendBuffer, plugin_name, MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
	}
}

DLL_EXPORT unsigned int GetUID()
{
	return plugin_uid;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	WSADATA ws;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
			WSAStartup(MAKEWORD(2, 2), &ws);
            break;
		case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
