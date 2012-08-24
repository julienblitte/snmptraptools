#include <windows.h>
#include "serviceController.h"
#include "..\core\snmptraptools_config.h"


SC_HANDLE hSCManager = NULL;
SC_HANDLE hService = NULL;

BOOL openService()
{
    hSCManager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, GENERIC_READ|GENERIC_EXECUTE);
    if (hSCManager == NULL)
    {
        return FALSE;
    }

    hService = OpenService(hSCManager, SERVICE_NAME, GENERIC_READ|GENERIC_EXECUTE);
    if (hService == NULL)
    {
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    return TRUE;
}

DWORD getServiceState()
{
    SERVICE_STATUS_PROCESS serviceInfo;
    DWORD sizeNeeded;

    if (hService == NULL)
    {
        return SERVICE_STOPPED;
    }

    if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE )&serviceInfo, sizeof(serviceInfo), &sizeNeeded) == 0)
    {
        return SERVICE_STOPPED;
    }

    return serviceInfo.dwCurrentState;
}

BOOL setServiceState(DWORD state)
{
    SERVICE_STATUS serviceStatus;

    if (hService == NULL)
    {
        return FALSE;
    }

    switch (state)
    {
        case SERVICE_CONTROL_START:
            return StartService(hService, 0, NULL);
            break;
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_PARAMCHANGE:
            return ControlService(hService, state, &serviceStatus);
            break;
    }
    return FALSE;
}

void closeService()
{
    if (hService != NULL)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }
    if (hSCManager != NULL)
    {
        CloseServiceHandle(hSCManager);
        hSCManager = NULL;
    }
}

