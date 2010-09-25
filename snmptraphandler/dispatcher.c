#include <windows.h>
#include <stdio.h>
#include "config.h"
#include "dispatcher.h"
#include "logger.h"

HANDLE gPipeReadHandle = NULL;
HANDLE gPipeWriteHandle = NULL;
HANDLE gDispatcherThread = NULL;

BOOL initDispatcher()
{
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    DWORD error;

    // set the bInheritHandle flag so pipe handles are inherited.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&gPipeReadHandle, &gPipeWriteHandle, &sa, 0))
    {
       error = GetLastError();
       logPrint(LOG_ERROR, "CreatePipe(initDispatcher) failed!");
       logPrintIntVar(LOG_ERROR, "GetLastError", error);
       return FALSE;
    }

    if (!SetHandleInformation(gPipeWriteHandle, HANDLE_FLAG_INHERIT, 0))
    {
        logPrint(LOG_ERROR, "SetHandleInformation(initDispatcher) failed!");
        return FALSE;
    }

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));

    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = gPipeReadHandle;   // stdin redirected
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (CreateProcess(NULL, DISPATCHER, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == FALSE)
    {
        error = GetLastError();
        logPrint(LOG_ERROR, "CreateProcess(initDispatcher) failed!");
        logPrintIntVar(LOG_ERROR, "GetLastError", error);
        return FALSE;
    }
    else
    {
        logPrint(LOG_DEBUG, "dispacher process is running.");
        logPrintIntVar(LOG_DEBUG, "pid", pi.dwProcessId);
    }

    gDispatcherThread = pi.hProcess;

    return TRUE;
}

void cleanDispatcher()
{
    CloseHandle(gPipeReadHandle);
    CloseHandle(gPipeWriteHandle);
}

DWORD printDispatcher(char *data)
{
    DWORD written;
    DWORD dispatcherExit;

    if (GetExitCodeProcess(gDispatcherThread, &dispatcherExit) == FALSE)
    {
        logPrint(LOG_ERROR, "GetExitCodeProcess failed!");
        return 0;
    }

    if (dispatcherExit != STILL_ACTIVE)
    {
        logPrint(LOG_ERROR, "Dispatcher process has exited! Trying to restart it...");

        cleanDispatcher();
        if (initDispatcher() == FALSE)
        {
            logPrint(LOG_ERROR, "initDispatcher failed!");
            return 0;
        }
        else
        {
            logPrint(LOG_INFORMATION, "Dispatcher restarted.");
        }
    }

    // asynchronous, do not test returned value - instead use written
    WriteFile(gPipeWriteHandle, data, strlen(data), &written, NULL);

    if (written != strlen(data))
    {
        logPrint(LOG_WARNING, "All information does not have been transmitted to dispatcher!");
    }

    return written;
}
