#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock2.h>
#include <ctype.h>

#include "plugin_http.h"
#include "plugin_http_resources.h"
#include "..\core\trapSnmp.h"
#include "..\core\plugin_common.h"

static const char plugin_name[] = "HTTP";
static const uint32_t plugin_uid = STR2UID('H','T','T','P');

static plugin_configuration config;

static HINSTANCE hinstance;


bool host_resolve(const char *host, SOCKADDR *resolved)
{
    struct addrinfo *resolution;
    struct addrinfo *result;
    int error;

    error = getaddrinfo(host, NULL, NULL, &resolution);
    if (error != 0)
    {
		return false;
    }

    for (result = resolution; result != NULL; result = result->ai_next)
    {
    	if (result->ai_addr->sa_family == AF_INET)
    	{
    		memcpy(resolved, result->ai_addr, sizeof(SOCKADDR));
    		freeaddrinfo(resolution);
    		return true;
    	}
    }

	freeaddrinfo(resolution);

	return false;
}

size_t url_encode(const char *str, char *out, size_t size_out)
{
	static const char hex[] = "0123456789abcdef";
	int i, j;

	i=0;
	j=0;
	while (j <= size_out)
	{
		if (isalnum(str[j]) || str[j] == '-' || str[j] == '_' || str[j] == '.' || str[j] == '~')
		{
			out[i++] = str[j];
		}
		else if (str[j] == ' ')
		{
			out[i++] = '+';
		}
		else if (str[j] == '\0')
		{
			out[i++] = str[j];
			break;
		}
		else
		{
			out[i++] = '%';
			out[i++] = hex[(str[j] >> 4) & 0x0F];
			out[i++] = hex[str[j] & 0x0F];
		}

		j++;
	}

	return i;
}

void push_http_parameter(char *p, size_t p_size, char *key, char *value)
{
	char encoded[256];
	char *header;

	url_encode(value, encoded, sizeof(encoded));
	if (*p == '\0')
	{
		snprintf(p, p_size, "%s=%s", key, encoded);
	}
	else
	{
		header = strdup(p);
		if (header != NULL)
		{
			snprintf(p, p_size, "%s&%s=%s", header, key, encoded);
			free(header);
		}
	}
}

bool http_post_request(const char *host, uint32_t port, const char *uri, const char *post_data)
{
	struct sockaddr_in dest;
	SOCKET sock;

	char buffer[2048];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET)
	{
		return false;
	}

	snprintf(buffer, sizeof(buffer), "POST %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %u\r\n\r\n%s",
			uri, host, "Snmptraptools", strlen(post_data), post_data);

#ifdef DEBUG
	printf("\nHTTP TRACE REQUEST:\n-------------------\n%s\n-------------------\n\n", buffer);
#endif
	
	if (!host_resolve(host, (SOCKADDR *)&dest))
	{
		printf("Error while trying resolving host %s.\n", host);
		return false;
	}
	dest.sin_port = htons(port);

	if(connect(sock, (SOCKADDR *)&dest, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("Error while trying to connect.\n");
		return false;
	}

	if (send(sock, buffer, strlen(buffer), 0) < 0)
	{
		printf("Error while trying to send.\n");
		return false;
	}

	while(recv(sock, buffer, sizeof(buffer), 0) < 0) {}

	closesocket(sock);

	return true;
}

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
		strncpy(config.address, "127.0.0.1", sizeof(config.address));
		config.port = 80;
		strncpy(config.script, "/index.php", sizeof(config.script));
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
	char sendBuffer[1024];
	char param[256];
	int i;

	strncpy(sendBuffer, "", sizeof(sendBuffer));

	snprintf(param, sizeof(param), "%u", trap->timestamp);
	push_http_parameter(sendBuffer, sizeof(sendBuffer), "TIMESTAMP", param);

	push_http_parameter(sendBuffer, sizeof(sendBuffer), "COMMUNITY", trap->community);

	snprintf(param, sizeof(param), "%u", trap->generic_type);
	push_http_parameter(sendBuffer, sizeof(sendBuffer), "GENERIC_TYPE", param);

	snprintf(param, sizeof(param), "%u", trap->specific_type);
	push_http_parameter(sendBuffer, sizeof(sendBuffer), "SPECIFIC_TYPE", param);

	push_http_parameter(sendBuffer, sizeof(sendBuffer), "ORIGIN", trap->agent);
	push_http_parameter(sendBuffer, sizeof(sendBuffer), "OID", trap->enterprise);

    for (i=0; i< trap->variables_count; i++)
    {
    	push_http_parameter(sendBuffer, sizeof(sendBuffer), trap->variables_oid[i], trap->variables_value[i]);
    }

    if (!http_post_request(config.address, config.port, config.script, sendBuffer))
    {
		i = GetLastError();
		snprintf(sendBuffer, sizeof(sendBuffer), "Error %d while sending message to %s.", i, config.address);
		MessageBox(0, sendBuffer, plugin_name, MB_OK|MB_ICONERROR|MB_SERVICE_NOTIFICATION);
    }

}

DLL_EXPORT uint32_t GETUID()
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
			hinstance = hinstDLL;
            break;
		case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL result;

    switch(uMsg)
    {
        case WM_INITDIALOG:
			SetDlgItemText(hDlg, ID_EDIT_IP_ADDRESS, config.address);
			SetDlgItemInt(hDlg, ID_EDIT_PORT, config.port, FALSE);
			SetDlgItemText(hDlg, ID_EDIT_SCRIPT, config.script);
            return TRUE;

        case WM_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;

        case WM_COMMAND:
			switch(wParam)
			{
				case ON_CLICK|ID_BUTTON_OK:
					GetDlgItemText(hDlg, ID_EDIT_IP_ADDRESS, config.address, sizeof(config.address));
					config.port = GetDlgItemInt(hDlg, ID_EDIT_PORT, &result, FALSE);
					if (!result || config.port > 65535)
					{
						MessageBox(hDlg, "Port is not valid!", "HTTP plugin", MB_ICONWARNING);
						SetDlgItemInt(hDlg, ID_EDIT_PORT, 80, FALSE);
					}
					GetDlgItemText(hDlg, ID_EDIT_SCRIPT, config.script, sizeof(config.script));
					EndDialog(hDlg, 0);
					return TRUE;

				case ON_CLICK|ID_BUTTON_CANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			return FALSE;
    }

    return FALSE;
}
