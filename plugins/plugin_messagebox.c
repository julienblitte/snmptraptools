#include <windows.h>
#include <stdint.h>

#include "..\core\trapSnmp.h"
#include "..\core\plugin_common.h"

static const char plugin_name[] = "messagebox";
static const uint32_t plugin_uid = STR2UID('M','S','G','B');

DLL_EXPORT const char *GETNAME()
{
	return plugin_name;
}

DLL_EXPORT void LOADCONFIG(const void *data, const unsigned int data_size)
{
	return;
}

DLL_EXPORT void *EDITCONFIG(void *data, unsigned int *data_size)
{
	static int config;

	// ask for default value template
	if (data == NULL)
	{
		// it's mandatory to send a valid pointer in this case
		*data_size = sizeof(config);
		return &config;
	}
	else
	{
		MessageBoxA(0, "No parameter available for this plugin.", plugin_name, MB_OK | MB_ICONINFORMATION);

		return NULL;
	}
}

DLL_EXPORT void RUN(snmpTrap *trap)
{
	static char buffer[1024];

	snprintf(buffer, sizeof(buffer), "%s\n\nagent: %s\noid: %s\ntype: %d", snmptrap_description(trap),
			trap->agent, trap->enterprise, snmptrap_code(trap));

	MessageBoxA(0, buffer, plugin_name, MB_OK | MB_ICONINFORMATION|MB_SERVICE_NOTIFICATION);
}

DLL_EXPORT uint32_t GETUID()
{
	return plugin_uid;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
