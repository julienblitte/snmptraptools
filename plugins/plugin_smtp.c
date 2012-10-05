#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>
#include <stdarg.h>

#include "plugin_smtp.h"
#include "plugin_smtp_resources.h"

#include "..\core\trapSnmp.h"
#include "..\core\plugin_common.h"

#define SMTP_PORT	25

static const char plugin_name[] = "E-mail";
static const uint32_t plugin_uid = STR2UID('S','M','T','P');

static plugin_configuration config;

static HINSTANCE hinstance;

static char smtp_io_buffer[1048];


#ifndef strcat_s
bool strcat_s(char *dest, size_t dest_size, const char *source)
{
	int i, j;

	i=0;
	while(dest[i])
		i++;

	if (i >= dest_size)
		return false;

	j = 0;
	while(i < dest_size-1)
	{
		dest[i] = source[j];
		i++;
		j++;
	}

	dest[i] = '\0';

	return true;
}
#endif

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

char *sgets(char *s, char *out, size_t out_size)
{
	int i, j;
	bool started;

	if (s == NULL)
		return NULL;

	i=0;
	j=0;
	started = false;
	while(i < out_size-1)
	{
		if (!started)
		{
			if (s[i] == '\0')
			{
				return NULL;
			}
			else if (s[i] == '\r' || s[i] == '\n')
			{
				// nothing
			}
			else
			{
				out[j] = s[i];
				j++;
				started = true;
			}
		}
		else
		{
			if (s[i] == '\0' || s[i] == '\r' || s[i] == '\n')
			{
				out[j] = '\0';
				j++;
				return &s[i];
			}
			else
			{
				out[j] = s[i];
				j++;
				started = true;
			}
		}
		i++;
	}
	return NULL;
}

uint32_t smtp_line_code(char *line)
{
	uint32_t result;

	result=0;
	while(*line != '\0')
	{
		switch(*line)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				result *= 10;
				result += (*line - '0');
				break;
			case ' ':
				return result;
			case '-':
			default:
				return 0;
		}
		line++;
	}
	return 0;
}

bool base64encode(const char *in, size_t in_size, char *out, size_t out_size)
{
	static const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const char padding = '=';
	int i;
	uint32_t pipe;

	i=0;
	pipe=0;
	while(i < in_size && out_size > 0)
	{
		// data in
		pipe <<= 8;
		pipe |=	in[i];
		i++;

		// data out
		if (i%3 == 0)
		{
			if (out_size < 4)
			{
				return false;
			}
			*out++ = base[(pipe>>18) & 63];
			*out++ = base[(pipe>>12) & 63];
			*out++ = base[(pipe>>6) & 63];
			*out++ = base[pipe & 63];

			out_size -= 4;
			pipe = 0;
		}
	}

	// pad remaining data in the pipe
	switch (i%3)
	{
		case 2:
			if (out_size < 4)	return false;	// aaaaaabbbbbbcccc00 + '='
			*out++ = base[(pipe>>10) & 63];
			*out++ = base[(pipe>>4) & 63];
			*out++ = base[(pipe<<2) & 63];
			*out++ = padding;
			out_size-=4;
			break;
		case 1:
			if (out_size < 4)	return false;	// aaaaaabb0000 + '=','='
			*out++ = base[(pipe>>2) & 63];
			*out++ = base[(pipe<<4) & 63];
			*out++ = padding;
			*out++ = padding;
			out_size-=4;
			break;
	}

	if (out_size < 1)	return false;
	*out++ = '\0';
	out_size--;

	return true;
}

uint32_t smtp_read(SOCKET sock)
{
	char line[256];
	uint32_t result;
	char *l;

#ifdef DEBUG
	memset(smtp_io_buffer, 0, sizeof(smtp_io_buffer));
#endif

	if (recv(sock, smtp_io_buffer, sizeof(smtp_io_buffer), 0) < 0)
	{
		return 0;
	}

#ifdef DEBUG
	printf("%s", smtp_io_buffer);
#endif

	l = sgets(smtp_io_buffer, line, sizeof(line));
	while(l != NULL)
	{
		result = smtp_line_code(line);

		if (result > 0)
		{
			return result;
		}
		l = sgets(l, line, sizeof(line));
	}

	return 0;
}

bool smtp_printf(SOCKET sock, const char *format, ...)
{
	va_list va;

    va_start(va, format);
	vsnprintf(smtp_io_buffer, sizeof(smtp_io_buffer)-3, format, va);
	va_end(va);

#ifdef DEBUG
	printf("%s\n", smtp_io_buffer);
#endif

	strcat(smtp_io_buffer, "\r\n");

	if (send(sock, smtp_io_buffer, strlen(smtp_io_buffer), 0) < 0)
	{
		return false;
	}

	return true;
}

#define SMTP_CHECK_STATUS(code)			if ((status = smtp_read(sock))/100 != code) { goto smtp_protocol_error; }
bool smtp_send(const char *host, const char *from, const char *to,
				const char *login, const char *password, const char *subject, const char *data)
{
	struct sockaddr_in dest;
	SOCKET sock;
	uint32_t status;

	char encoded[192];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET)
	{
		return false;
	}

	if (!host_resolve(host, (SOCKADDR *)&dest))
	{
		printf("Error while trying resolving host %s.\n", host);
		return false;
	}
	dest.sin_port = htons(SMTP_PORT);

	if(connect(sock, (SOCKADDR *)&dest, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("Error while trying to connect.\n");
		return false;
	}


#ifdef DEBUG
	printf("--------------------- SMTP starts ---------------------\n");
#endif
	SMTP_CHECK_STATUS(2)		// welcome

	smtp_printf(sock, "EHLO %s", host);

	SMTP_CHECK_STATUS(2)		// nice to meet you

	if (login != NULL && password != NULL)
	{
		smtp_printf(sock, "AUTH LOGIN");
		SMTP_CHECK_STATUS(3)		// login?

		base64encode(login, strlen(login), encoded, sizeof(encoded));
		smtp_printf(sock, "%s", encoded);
		SMTP_CHECK_STATUS(3)		// password?

		base64encode(password, strlen(password), encoded, sizeof(encoded));
		smtp_printf(sock, "%s", encoded);
		SMTP_CHECK_STATUS(2)		// authenticated
	}

	smtp_printf(sock, "MAIL FROM: <%s>", from);
	SMTP_CHECK_STATUS(2)		// sender ok

	smtp_printf(sock, "RCPT TO: <%s>", to);
	SMTP_CHECK_STATUS(2)		// recipient ok

	smtp_printf(sock, "DATA");
	SMTP_CHECK_STATUS(3)		// type content

	// mail header
	smtp_printf(sock, "From: \"%s\" <%s>", from, from);
	smtp_printf(sock, "To: \"%s\" <%s>", to, to);
	smtp_printf(sock, "X-Mailer: Snmptraptools - SMTP plugin");
	smtp_printf(sock, "Subject: %s", subject);
	smtp_printf(sock, "");

	// mail content
	smtp_printf(sock, "%s", data);

	smtp_printf(sock, ".");
	SMTP_CHECK_STATUS(2)		// Finished

	smtp_printf(sock, "QUIT");
	SMTP_CHECK_STATUS(2)		// bye

	closesocket(sock);

#ifdef DEBUG
	printf("--------------------- SMTP stops ---------------------\n");
#endif
	return true;


smtp_protocol_error:
#ifdef DEBUG
	printf("--------------------- SMTP error ---------------------\n");
#endif
	closesocket(sock);
	return false;
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
		strncpy(config.server, "127.0.0.1", sizeof(config.server));
		strncpy(config.from, "sender@email.com", sizeof(config.from));
		strncpy(config.to, "reciever@email.com", sizeof(config.to));
		config.authentication = false;
		strncpy(config.login, "", sizeof(config.login));
		strncpy(config.password, "", sizeof(config.password));

		*data_size = sizeof(config);
	}
	else
	{
		LOADCONFIG(data, *data_size);
		DialogBox(hinstance, MAKEINTRESOURCE(ID_DIALOG_CONFIG), NULL, (DLGPROC)dlgAddMessageHandler);
	}

	return &config;
}

DLL_EXPORT void RUN(snmpTrap *trap)
{
	char buffer[2048];
	char param[512];
	int i;

	snprintf(buffer, sizeof(buffer), "%s\r\n\r\nAgent: %s\r\nOID: %s\r\nGeneric type: %u\r\n"
			"Specific type: %u\r\nCommunity: %s\r\n", snmptrap_description(trap), trap->agent,
			trap->enterprise, trap->generic_type, trap->specific_type, trap->community);

    for (i=0; i< trap->variables_count; i++)
    {
    	snprintf(param, sizeof(param), "\r\n%s: %s", trap->variables_oid[i], trap->variables_value[i]);
    	strcat_s(buffer, sizeof(buffer), param);
    }

	if (config.authentication)
	{
		smtp_send(config.server, config.from, config.to, config.login, config.password, snmptrap_description(trap), buffer);
	}
	else
	{
		smtp_send(config.server, config.from, config.to, NULL, NULL, snmptrap_description(trap), buffer);
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

void enableAuthentication(HWND hDlg, bool enabled)
{
    HWND hwnd[2];

    hwnd[0] = GetDlgItem(hDlg, ID_EDIT_LOGIN);
    hwnd[1] = GetDlgItem(hDlg, ID_EDIT_PASSWORD);

    if (hwnd[0] != NULL && hwnd[1] != NULL)
    {
        EnableWindow(hwnd[0], enabled);
        EnableWindow(hwnd[1], enabled);
    }

    return;
}

BOOL CALLBACK dlgAddMessageHandler(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
			SetDlgItemText(hDlg, ID_EDIT_SERVER, config.server);
			SetDlgItemText(hDlg, ID_EDIT_FROM, config.from);
			SetDlgItemText(hDlg, ID_EDIT_TO, config.to);
			if (config.authentication)
			{
				CheckDlgButton(hDlg, ID_CHECKBOX_AUTH, BST_CHECKED);
				SetDlgItemText(hDlg, ID_EDIT_LOGIN, config.login);
				SetDlgItemText(hDlg, ID_EDIT_PASSWORD, config.password);
				enableAuthentication(hDlg, true);
			}
			else
			{
				CheckDlgButton(hDlg, ID_CHECKBOX_AUTH, BST_UNCHECKED);
				SetDlgItemText(hDlg, ID_EDIT_LOGIN, "");
				SetDlgItemText(hDlg, ID_EDIT_PASSWORD, "");
				enableAuthentication(hDlg, false);
			}
            return TRUE;

        case WM_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;

        case WM_COMMAND:
			switch(wParam)
			{
				case ON_CLICK|ID_BUTTON_OK:
					GetDlgItemText(hDlg, ID_EDIT_SERVER, config.server, sizeof(config.server));
					GetDlgItemText(hDlg, ID_EDIT_FROM, config.from, sizeof(config.from));
					GetDlgItemText(hDlg, ID_EDIT_TO, config.to, sizeof(config.to));

					if (IsDlgButtonChecked(hDlg, ID_CHECKBOX_AUTH) == BST_CHECKED)
					{
						config.authentication = true;
						GetDlgItemText(hDlg, ID_EDIT_LOGIN, config.login, sizeof(config.login));
						GetDlgItemText(hDlg, ID_EDIT_PASSWORD, config.password, sizeof(config.password));
					}
					else
					{
						config.authentication = false;
						strncpy(config.login, "", sizeof(config.login));
						strncpy(config.password, "", sizeof(config.password));
					}

					EndDialog(hDlg, 0);
					return TRUE;

				case ON_CLICK|ID_BUTTON_CANCEL:
					EndDialog(hDlg, 0);
					return TRUE;

				case ON_CHECK|ID_CHECKBOX_AUTH:
					enableAuthentication(hDlg, (IsDlgButtonChecked(hDlg, ID_CHECKBOX_AUTH) == BST_CHECKED));
					break;
			}
			return FALSE;
    }

    return FALSE;
}
