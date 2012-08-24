#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "..\core\snmptraptools_config.h"
#include "dispatcher.h"
#include "logger.h"
#include "service.h"

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
       logPrintf(LOG_ERROR, "CreatePipe(initDispatcher) failed!\n");
       logPrintf(LOG_ERROR, "GetLastError: %d\n", error);
       return FALSE;
    }

    if (!SetHandleInformation(gPipeWriteHandle, HANDLE_FLAG_INHERIT, 0))
    {
        logPrintf(LOG_ERROR, "SetHandleInformation(initDispatcher) failed!\n");
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
        logPrintf(LOG_ERROR, "CreateProcess(initDispatcher) failed!\n");
        logPrintf(LOG_ERROR, "GetLastError: %d\n", error);
        return FALSE;
    }
    else
    {
        logPrintf(LOG_DEBUG, "dispacher process running, pid: %u\n", pi.dwProcessId);
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
        logPrintf(LOG_ERROR, "GetExitCodeProcess failed!\n");
        return 0;
    }

    if (dispatcherExit != STILL_ACTIVE)
    {
        logPrintf(LOG_ERROR, "Dispatcher process has exited! Trying to restart it...\n");

        cleanDispatcher();
        if (initDispatcher() == FALSE)
        {
            logPrintf(LOG_ERROR, "initDispatcher failed!\n");
            return 0;
        }
        else
        {
            logPrintf(LOG_INFORMATION, "Dispatcher restarted.\n");
        }
    }

    // asynchronous, do not test returned value - instead use written
    WriteFile(gPipeWriteHandle, data, strlen(data), &written, NULL);

    logPrintf(LOG_PEDANTIC, "dispatcher/%s", data);

    if (written != strlen(data))
    {
        logPrintf(LOG_WARNING, "All information does not have been transmitted to dispatcher!\n");
    }

    return written;
}

DWORD printDispatcherF(char *format, ...)
{
    va_list va;
    static char dispatcher_buffer[MAX_DISPATCHER_LINE_LEN];

    va_start(va, format);

    vsnprintf(dispatcher_buffer, sizeof(dispatcher_buffer), format, va);

    return printDispatcher(dispatcher_buffer);
}
