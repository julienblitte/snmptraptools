#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <winsnmp.h>
#include "service.h"
#include "dispatcher.h"
#include "config.h"
#include "logger.h"
#include "snmp.h"

HSNMP_SESSION hSnmpSession;

void safeDispatchData(char *data)
{
    if (data == NULL)
    {
        logPrint(LOG_WARNING, "safeDispatchData called with a NULL pointer!");
        return;
    }

    // empty string
    if (data[0] == '\0')
    {
        return;
    }

    if (printDispatcher(data) == 0)
    {
        logPrint(LOG_ERROR, "printDispatcher failed!");
        PanicQuitService(ERROR_DISPATCHER_WRITE);
    }
}


SNMPAPI_STATUS snmpMessage(HSNMP_SESSION hSession, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam, LPVOID lpClientData)
{
    // data
    HSNMP_CONTEXT hContext;
    HSNMP_PDU data;

    // decoded content
    INT PDU_type;
    INT32 request_id;
    INT error_status;
    INT error_index;
    HSNMP_VBL varbindlist;

    smiOCTETS context;

    UINT32 variableIndex, variableCount;

    // variables found in content
    smiOID oid;
    smiVALUE value;

    // dispatcher buffer
    static char dispatcherBuffer[MAX_DISPATCHER_LINE_LEN];

    if (wParam == 0)
    {
        logPrint(LOG_DEBUG, "Data recieved from snmp stack");

        if (SnmpRecvMsg(hSession, NULL, NULL, &hContext, &data) != SNMPAPI_SUCCESS)
        {
            logPrint(LOG_ERROR, "SnmprecvMsg failed!");
            return SNMPAPI_FAILURE;
        }

        // decode content
        if (SnmpGetPduData(data, &PDU_type, &request_id, &error_status, &error_index, &varbindlist) != SNMPAPI_SUCCESS)
        {
            logPrint(LOG_ERROR, "SnmpGetPduData failed!");
            return SNMPAPI_FAILURE;
        }

        if (PDU_type == SNMP_PDU_TRAP)
        {
            // get variable list
            variableCount = SnmpCountVbl(varbindlist);
            if (variableCount == SNMPAPI_FAILURE)
            {
                logPrint(LOG_ERROR, "SnmpCountVbl failed!");
                return SNMPAPI_FAILURE;
            }

            // retrieve community and write it like other variables
            SnmpContextToStr(hContext, &context);

            if (context.len < (sizeof(dispatcherBuffer)-1))
            {
                memcpy(dispatcherBuffer, context.ptr, context.len);
                dispatcherBuffer[context.len] = '\0';
            }
            safeDispatchData("1.3.6.1.6.3.1.2.2.9.0\t");
            safeDispatchData(dispatcherBuffer);
            safeDispatchData("\n");

            // other stuff
            for(variableIndex=1; variableIndex <= variableCount; variableIndex++)
            {
                if (SnmpGetVb(varbindlist, variableIndex, &oid, &value) != SNMPAPI_SUCCESS)
                {
                    logPrint(LOG_ERROR, "SnmpGetVb failed!");
                    return SNMPAPI_FAILURE;
                }

                if (SnmpOidToStr(&oid, sizeof(dispatcherBuffer), dispatcherBuffer) == SNMPAPI_FAILURE)
                {
                    logPrint(LOG_ERROR, "SnmpOidToStr failed!");
                    snprintf(dispatcherBuffer, sizeof(dispatcherBuffer), "-");
                }

                safeDispatchData(dispatcherBuffer);
                safeDispatchData("\t");

                if (SnmpValueToStr(&value, dispatcherBuffer, sizeof(dispatcherBuffer)) == SNMPAPI_FAILURE)
                {
                    logPrint(LOG_ERROR, "SnmpValueToStr failed!");
                    snprintf(dispatcherBuffer, sizeof(dispatcherBuffer), "-");
                }

                safeDispatchData(dispatcherBuffer);
                safeDispatchData("\n");

                SnmpFreeOID(&oid);
                SnmpFreeOCTETS(&context);
                SnmpFreeValue(&value);
            }

            // blank line to inform user that all packet is done
            safeDispatchData("\n");
        }
        else
        {
            logPrint(LOG_WARNING, "SNMP packet recieved but not a TRAP.");
        }

        //SnmpFreeEntity(hSource);
        //SnmpFreeEntity(hDestination);
        SnmpFreeContext(hContext);
        SnmpFreePdu(data);
    }

    return SNMPAPI_SUCCESS;
}

void MyServiceContinue()
{
    logPrint(LOG_DEBUG, "Try to continue service...");
    if (SnmpRegister(hSnmpSession, NULL, NULL, NULL, NULL, SNMPAPI_ON) == SNMPAPI_FAILURE)
    {
        logPrint(LOG_ERROR, "SnmpRegister failed during enabling!");
    }
    logPrint(LOG_DEBUG, "Service continued.");
}

void MyServicePause()
{
    logPrint(LOG_DEBUG, "Try to pause service...");
    if (SnmpRegister(hSnmpSession, NULL, NULL, NULL, NULL, SNMPAPI_OFF) == SNMPAPI_FAILURE)
    //SNMP_PDU_TRAP
    {
        logPrint(LOG_ERROR, "SnmpRegister failed during disabling!");
    }
    logPrint(LOG_DEBUG, "Service paused.");
}

void MyServiceStart()
{
    logPrint(LOG_DEBUG, "Try to start service...");

    hSnmpSession = SnmpCreateSession(NULL, 0, (SNMPAPI_CALLBACK)snmpMessage, NULL);
    if (hSnmpSession == SNMPAPI_FAILURE)
    {
        logPrint(LOG_ERROR, "SnmpCreateSession failed!");
        PanicQuitService(ERROR_SNMP_INIT);
        return;
    }

    if (initDispatcher() == false)
    {
        logPrint(LOG_ERROR, "initDispatcher failed!");
        PanicQuitService(ERROR_DISPATCHER_INIT);
        return;
    }

    logPrint(LOG_DEBUG, "Service is ready.");

    // To enable trap handling
    MyServiceContinue();
}

void MyServiceStop()
{
    logPrint(LOG_DEBUG, "Try to stop service...");
    // this will close pipe between service and child process
    // child process should terminate by it self (no more data on STDIN)
    cleanDispatcher();

    if (SnmpClose(hSnmpSession) != SNMPAPI_SUCCESS)
    {
        logPrint(LOG_ERROR, "SnmpClose failed!");
        return;
    }
    logPrint(LOG_DEBUG, "Service stopped.");
}

void MyServiceReload()
{
    logPrint(LOG_DEBUG, "Reloading dispatcher...");
    cleanDispatcher();

    if (initDispatcher() == false)
    {
        logPrint(LOG_ERROR, "initDispatcher failed!");
        PanicQuitService(ERROR_DISPATCHER_INIT);
        return;
    }

    logPrint(LOG_INFORMATION, "Dispatcher reloaded.");
}

int main( int argc, char *argv[])
{
    service myService;
    smiUINT32 snmpStackInfo[5];

    myService.name = SERVICE_NAME;
    myService.description = SERVICE_DESC;
    myService.dependencies = "SNMPTRAP\0""EventLog\0";
    myService.onContinue = &MyServiceContinue;
    myService.onPause = &MyServicePause;
    myService.onStart = &MyServiceStart;
    myService.onStop = &MyServiceStop;
    myService.onReload = &MyServiceReload;

    logInit();

    if (argc > 2)
    {
        // printf("Usage: %s [ install | uninstall | non-service-mode ]\n", argv[0]);
        printf("Usage: %s [ install | uninstall ]\n", argv[0]);
        return EXIT_FAILURE;
    }

    InitService(&myService);

    if (argc == 2 && ! stricmp(argv[1], "install"))
    {
        logPrint(LOG_INFORMATION, "Installing service...");
        InstallService();
        logPrint(LOG_INFORMATION, "Service installed.");
        return EXIT_SUCCESS;
    }
    else if (argc == 2 && ! stricmp(argv[1], "uninstall"))
    {
        logPrint(LOG_INFORMATION, "Uninstalling service...");
        UninstallService();
        logPrint(LOG_INFORMATION, "Service uninstalled.");
        logPrint(LOG_WARNING, "User must restart system for changes to take effect.");
        return EXIT_SUCCESS;
    }
    else if (argc == 2 && ! stricmp(argv[1], "non-service-mode"))
    {
        logPrint(LOG_DEBUG, "SNMP stack initialization...");
        if (SnmpStartup(&snmpStackInfo[0], &snmpStackInfo[1], &snmpStackInfo[2],
                        &snmpStackInfo[3], &snmpStackInfo[4]) != SNMPAPI_SUCCESS)
        {
            logPrint(LOG_ERROR, "SnmpStartup failed!");
            return EXIT_FAILURE;
        }

        // ici, gérer les messages
        MyServiceStart();

        printf("started and running.\n");
        char c = '\0';
        while(c != 's' && c != 'S')
        {
            printf("Pause, Continue or Stop? [p,c,s]");
            c = getchar();
            while (getchar() != '\n') ;

            switch(c)
            {
                case 'p':
                case 'P':
                    MyServicePause();
                    printf("paused.\n");
                    break;
                case 'c':
                case 'C':
                    MyServiceContinue();
                    printf("running.\n");
                    break;
            }
        }

        MyServiceStop();

        SnmpCleanup();
    }
    else
    {
        logPrint(LOG_INFORMATION, "Starting service...");
        logPrint(LOG_DEBUG, "SNMP stack initialization...");
        logPrintPtrVar("snmpStackInfo", snmpStackInfo);
        if (SnmpStartup(&snmpStackInfo[0], &snmpStackInfo[1], &snmpStackInfo[2],
                        &snmpStackInfo[3], &snmpStackInfo[4]) != SNMPAPI_SUCCESS)
        {
            logPrint(LOG_ERROR, "SnmpStartup failed!");
            return EXIT_FAILURE;
        }
        logPrint(LOG_DEBUG, "Windows service registering...");
        RunService();
        logPrint(LOG_INFORMATION, "Service terminated.");
        SnmpCleanup();
    }

    return EXIT_SUCCESS;
}
