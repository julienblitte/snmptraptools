#include <windows.h>
#include <stdio.h>
#include "service.h"
#include "logger.h"

#define PENDING_WAIT    3000

// Some global vars
SERVICE_STATUS          gStatus;
SERVICE_STATUS_HANDLE   gStatusHandle;
HANDLE                  ghStopEvent;
service                 gService;
DWORD                   gServiceAccepted;

void InstallService()
{
    SC_HANDLE hSCMgr;
    SC_HANDLE hService;
    TCHAR ExeAddr[MAX_PATH];
    SERVICE_DESCRIPTION sd;

    if (! GetModuleFileName(NULL, ExeAddr, MAX_PATH)) {
        logPrint(LOG_ERROR, "GetModuleFileName failed!");
        return;
    }
    else
    {
        logPrint(LOG_DEBUG, "Executable is:");
        logPrint(LOG_DEBUG, ExeAddr);
    }

    hSCMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCMgr == NULL) {
        logPrint(LOG_ERROR, "OpenSCManager failed!");
        return;
    }
    else
    {
        logPrint(LOG_DEBUG, "OpenSCManager done.");
    }

    hService = CreateService(hSCMgr, gService.name, gService.name, SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        ExeAddr, NULL, NULL, gService.dependencies, NULL, NULL);

    if (hService == NULL)
    {
        logPrint(LOG_ERROR, "CreateService failed!");
        CloseServiceHandle(hSCMgr);
        return;
    }
    else
    {
        logPrint(LOG_DEBUG, "CreateService is ok.");
    }

    // Now, to change its description
    sd.lpDescription = (char *)gService.description;
    if (! ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &sd))
    {
        logPrint(LOG_ERROR, "ChangeServiceConfig2 failed!");
    }
    else
    {
        logPrint(LOG_DEBUG, "ChangeServiceConfig2 is ok.");
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCMgr);
}

void UninstallService()
{
    SC_HANDLE hSCMgr;
    SC_HANDLE hService;

    hSCMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    logPrintPtrVar("hSCMgr", hSCMgr);
    if (NULL == hSCMgr)
    {
        logPrint(LOG_ERROR, "OpenSCManager failed!");
        return;
    }
    else
    {
        logPrint(LOG_DEBUG, "OpenSCManager done.");
    }

    hService = OpenService(hSCMgr, gService.name, DELETE);
    logPrintPtrVar("hService", hService);

    if (hService == NULL)
    {
        logPrint(LOG_ERROR, "OpenService failed!");
        CloseServiceHandle(hSCMgr);
        return;
    }
    else
    {
        logPrint(LOG_DEBUG, "OpenService done.");
    }

    if (! DeleteService(hService))
    {
        logPrint(LOG_ERROR, "DeleteService Failed!");
    }
    else
    {
        logPrint(LOG_DEBUG, "DeleteService is ok.");
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCMgr);
}

DWORD GetStatus()
{
    return gStatus.dwCurrentState;
}

void SetStatus(DWORD State)
{
    gStatus.dwCurrentState = State;

    // service is starting, no action allowed on int
    if (State == SERVICE_START_PENDING)
    {
        gStatus.dwControlsAccepted = 0;
    }
    else
    {
        gStatus.dwControlsAccepted = gServiceAccepted;
    }

    // service is busy, try to estimate delay
    if (State == SERVICE_CONTINUE_PENDING || State == SERVICE_PAUSE_PENDING ||
        State == SERVICE_START_PENDING || State == SERVICE_STOP_PENDING)
    {
        gStatus.dwWaitHint = PENDING_WAIT;
    }
    else
    {
        gStatus.dwWaitHint = 0;
    }

    SetServiceStatus(gStatusHandle, &gStatus);
}

void WINAPI CtrlHandler(DWORD CtrlCmd)
{
    switch (CtrlCmd)
    {
        case SERVICE_CONTROL_PAUSE:
            SetStatus(SERVICE_PAUSE_PENDING);
            gService.onPause();
            SetStatus(SERVICE_PAUSED);
            break;
        case SERVICE_CONTROL_CONTINUE:
            SetStatus(SERVICE_CONTINUE_PENDING);
            gService.onContinue();
            SetStatus(SERVICE_RUNNING);
            break;
        case SERVICE_CONTROL_STOP:
            SetStatus(SERVICE_STOP_PENDING);
            gService.onStop();
            SetStatus(SERVICE_STOPPED);
            SetEvent(ghStopEvent);
            break;
        case SERVICE_CONTROL_PARAMCHANGE:
            SetStatus(SERVICE_CONTINUE_PENDING);
            gService.onReload();
            SetStatus(SERVICE_RUNNING);
            break;
        case SERVICE_CONTROL_INTERROGATE:
        default:
            SetStatus(GetStatus());
            break;
    }
}

void MainService()
{
    int error;

    // service registers
    gStatusHandle = RegisterServiceCtrlHandler(gService.name, CtrlHandler);
    if (gStatusHandle == 0)
    {

        error = GetLastError();
        logPrint(LOG_ERROR, "RegisterServiceCtrlHandler failed!");
        logPrintIntVar(LOG_ERROR, "GetLastError", error);
        return;
    }

    gStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gStatus.dwServiceSpecificExitCode = 0;
    gStatus.dwWin32ExitCode = 0;

    ghStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    logPrintPtrVar("ghStopEvent", ghStopEvent);
    if (ghStopEvent == NULL)
    {
        SetStatus(SERVICE_STOPPED);
        logPrint(LOG_ERROR, "CreateEvent failed!");
        return;
    }

    // service starts
    SetStatus(SERVICE_START_PENDING);
    gService.onStart();
    SetStatus(SERVICE_RUNNING);

    logPrint(LOG_INFORMATION, "Serice is running.");

    // service is running until ghStopEvent is set
    WaitForSingleObject(ghStopEvent, INFINITE);

    // stops service if not (can be already stopped by another way (e.g. panicMode)
    if (gStatus.dwCurrentState != SERVICE_STOPPED)
    {
        SetStatus(SERVICE_STOPPED);
    }
}

void RunService()
{
    SERVICE_TABLE_ENTRY dispatchTable[2];

    dispatchTable[0].lpServiceName = (char *)gService.name;
    dispatchTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)&MainService;

    dispatchTable[1].lpServiceName = NULL;
    dispatchTable[1].lpServiceProc = NULL;

    logPrintPtrVar("dispatchTable", dispatchTable);

    // here non return-function
    if (!StartServiceCtrlDispatcher(dispatchTable))
    {
        logPrint(LOG_ERROR, "StartServiceCtrlDispatcher failed!");
    }

}

void InitService(service *Service)
{
    gService = *Service;

    logPrintPtrVar("Service", Service);
    logPrintPtrVar("gService", &gService);

    gServiceAccepted = 0;
    if ((gService.onContinue != NULL) && (gService.onPause != NULL))
    {
        gServiceAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    }
    if (gService.onStop != NULL)
    {
        gServiceAccepted |= SERVICE_ACCEPT_STOP;
    }
    if (gService.onReload != NULL)
    {
        gServiceAccepted |= SERVICE_ACCEPT_PARAMCHANGE;
    }
}


void StopService()
{
    SetEvent(ghStopEvent);
}

void PanicQuitService(DWORD errorCode)
{
    gStatus.dwCurrentState = SERVICE_STOPPED;
    gStatus.dwControlsAccepted = 0;
    gStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    gStatus.dwServiceSpecificExitCode = errorCode;
    gStatus.dwWaitHint = 0;

    SetServiceStatus(gStatusHandle, &gStatus);
    SetEvent(ghStopEvent);
}
