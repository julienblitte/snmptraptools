#include "plugin_messagebox.h"
#include "..\core\trapSnmp.h"
#include <stdbool.h>

static const char plugin_name[] = PLUGIN_NAME;
static unsigned int plugin_uid = PLUGIN_UID;

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
	MessageBoxA(0, "No parameter available for this plugin.", plugin_name, MB_OK | MB_ICONINFORMATION);

	return NULL;
}

DLL_EXPORT void Run(snmpTrap *trap)
{
	static char buffer[1024];

	snprintf(buffer, sizeof(buffer), "%s\n\nagent: %s\noid: %s\ntype: %d", snmptrap_description(trap),
			trap->agent, trap->enterprise, snmptrap_code(trap));

	MessageBoxA(0, buffer, plugin_name, MB_OK | MB_ICONINFORMATION|MB_SERVICE_NOTIFICATION);
}

DLL_EXPORT unsigned int GetUID()
{
	return plugin_uid;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	/*
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    */
    return TRUE;
}
