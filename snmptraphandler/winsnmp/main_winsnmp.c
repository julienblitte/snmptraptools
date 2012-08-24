#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <winsnmp.h>
#include "..\service.h"
#include "..\dispatcher.h"
#include "..\..\core\snmptraptools_config.h"
#include "..\logger.h"
#include "snmp.h"

HSNMP_SESSION hSnmpSession;

SNMPAPI_STATUS snmpMessage(HSNMP_SESSION hSession, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam, LPVOID lpClientData)
{
    // data
    HSNMP_CONTEXT hContext;
    static char dispatcherBuffer[1024];
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

    if (wParam == 0)
    {
        logPrintf(LOG_DEBUG, "Data recieved from snmp stack\n");

        if (SnmpRecvMsg(hSession, NULL, NULL, &hContext, &data) != SNMPAPI_SUCCESS)
        {
            logPrintf(LOG_ERROR, "SnmprecvMsg failed!\n");
            return SNMPAPI_FAILURE;
        }

        // decode content
        if (SnmpGetPduData(data, &PDU_type, &request_id, &error_status, &error_index, &varbindlist) != SNMPAPI_SUCCESS)
        {
            logPrintf(LOG_ERROR, "SnmpGetPduData failed!\n");
            return SNMPAPI_FAILURE;
        }

        if (PDU_type == SNMP_PDU_TRAP)
        {
            // get variable list
            variableCount = SnmpCountVbl(varbindlist);
            if (variableCount == SNMPAPI_FAILURE)
            {
                logPrintf(LOG_ERROR, "SnmpCountVbl failed!\n");
                return SNMPAPI_FAILURE;
            }

            // retrieve community and write it like other variables
            SnmpContextToStr(hContext, &context);
            if (context.len < (sizeof(dispatcherBuffer)-1))
            {
                memcpy(dispatcherBuffer, context.ptr, context.len);
                dispatcherBuffer[context.len] = '\0';
            }
            else
            {
                memcpy(dispatcherBuffer, context.ptr, (sizeof(dispatcherBuffer)-1));
                dispatcherBuffer[(sizeof(dispatcherBuffer)-1)] = '\0';
            }
            printDispatcherF("%s\t%s\n", OID_COMMUNITY, dispatcherBuffer);
            SnmpFreeOCTETS(&context);

            // other stuff
            for(variableIndex=1; variableIndex <= variableCount; variableIndex++)
            {
                if (SnmpGetVb(varbindlist, variableIndex, &oid, &value) != SNMPAPI_SUCCESS)
                {
                    logPrintf(LOG_ERROR, "SnmpGetVb failed!\n");
                    return SNMPAPI_FAILURE;
                }

                if (SnmpOidToStr(&oid, sizeof(dispatcherBuffer), dispatcherBuffer) == SNMPAPI_FAILURE)
                {
                    logPrintf(LOG_ERROR, "SnmpOidToStr failed!\n");
                }
                else
                {
                    SnmpFreeOID(&oid);

                    printDispatcherF("%s\t", dispatcherBuffer);

                    if (SnmpValueToStr(&value, dispatcherBuffer, sizeof(dispatcherBuffer)) == SNMPAPI_FAILURE)
                    {
                        logPrintf(LOG_ERROR, "SnmpValueToStr failed!\n");
                        printDispatcherF("-\n");
                    }
                    else
                    {
                        SnmpFreeValue(&value);
                        printDispatcherF("%s\n", dispatcherBuffer);
                    }
                }
            }

            // blank line to inform user that all packet is done
            printDispatcherF("\n");
        }
        else
        {
            logPrintf(LOG_WARNING, "SNMP packet recieved but not a TRAP.\n");
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
    logPrintf(LOG_DEBUG, "Try to continue service...\n");
    if (SnmpRegister(hSnmpSession, NULL, NULL, NULL, NULL, SNMPAPI_ON) == SNMPAPI_FAILURE)
    {
        logPrintf(LOG_ERROR, "SnmpRegister failed during enabling!\n");
    }
    logPrintf(LOG_INFORMATION, "Service continued.\n");
}

void MyServicePause()
{
    logPrintf(LOG_DEBUG, "Try to pause service...\n");
    if (SnmpRegister(hSnmpSession, NULL, NULL, NULL, NULL, SNMPAPI_OFF) == SNMPAPI_FAILURE)
    //SNMP_PDU_TRAP
    {
        logPrintf(LOG_ERROR, "SnmpRegister failed during disabling!\n");
    }
    logPrintf(LOG_INFORMATION, "Service paused.\n");
}

void MyServiceStart()
{
    logPrintf(LOG_DEBUG, "Try to start service...\n");

    hSnmpSession = SnmpCreateSession(NULL, 0, (SNMPAPI_CALLBACK)snmpMessage, NULL);
    if (hSnmpSession == SNMPAPI_FAILURE)
    {
        logPrintf(LOG_ERROR, "SnmpCreateSession failed!\n");
        PanicQuitService(ERROR_SNMP_INIT);
        return;
    }

    if (initDispatcher() == false)
    {
        logPrintf(LOG_ERROR, "initDispatcher failed!\n");
        PanicQuitService(ERROR_DISPATCHER_INIT);
        return;
    }

    logPrintf(LOG_INFORMATION, "Service is ready.\n");

    // To enable trap handling
    MyServiceContinue();
}

void MyServiceStop()
{
    logPrintf(LOG_DEBUG, "Try to stop service...\n");
    // this will close pipe between service and child process
    // child process should terminate by it self (no more data on STDIN)
    cleanDispatcher();

    if (SnmpClose(hSnmpSession) != SNMPAPI_SUCCESS)
    {
        logPrintf(LOG_ERROR, "SnmpClose failed!\n");
        return;
    }
    logPrintf(LOG_INFORMATION, "Service stopped.\n");
}

void MyServiceReload()
{
    logPrintf(LOG_DEBUG, "Reloading dispatcher...\n");
    cleanDispatcher();

    if (initDispatcher() == false)
    {
        logPrintf(LOG_ERROR, "initDispatcher failed!\n");
        PanicQuitService(ERROR_DISPATCHER_INIT);
        return;
    }

    logPrintf(LOG_INFORMATION, "Dispatcher reloaded.\n");
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
        logPrintf(LOG_INFORMATION, "Installing service...\n");
        InstallService();
        logPrintf(LOG_INFORMATION, "Service installed.\n");
        return EXIT_SUCCESS;
    }
    else if (argc == 2 && ! stricmp(argv[1], "uninstall"))
    {
        logPrintf(LOG_INFORMATION, "Uninstalling service...\n");
        UninstallService();
        logPrintf(LOG_INFORMATION, "Service uninstalled.\n");
        logPrintf(LOG_WARNING, "User must restart system for changes to take effect.\n");
        return EXIT_SUCCESS;
    }
    else if (argc == 2 && ! stricmp(argv[1], "non-service-mode"))
    {
        logPrintf(LOG_DEBUG, "SNMP stack initialization...\n");
        if (SnmpStartup(&snmpStackInfo[0], &snmpStackInfo[1], &snmpStackInfo[2],
                        &snmpStackInfo[3], &snmpStackInfo[4]) != SNMPAPI_SUCCESS)
        {
            logPrintf(LOG_ERROR, "SnmpStartup failed!\n");
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
        logPrintf(LOG_INFORMATION, "Starting service...\n");
        logPrintf(LOG_DEBUG, "SNMP stack initialization...\n");

        if (SnmpStartup(&snmpStackInfo[0], &snmpStackInfo[1], &snmpStackInfo[2],
                        &snmpStackInfo[3], &snmpStackInfo[4]) != SNMPAPI_SUCCESS)
        {
            logPrintf(LOG_ERROR, "SnmpStartup failed!\n");
            return EXIT_FAILURE;
        }
        logPrintf(LOG_DEBUG, "Windows service registering...\n");
        RunService();
        logPrintf(LOG_INFORMATION, "Service terminated.\n");
        SnmpCleanup();
    }

    return EXIT_SUCCESS;
}
