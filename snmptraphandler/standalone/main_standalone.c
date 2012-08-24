#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "..\service.h"
#include "..\dispatcher.h"
#include "..\..\core\snmptraptools_config.h"
#include "..\logger.h"
#include "snmp_stack.h"
#include <winsock2.h>


DWORD WINAPI udp_server(LPVOID lpParam);
HANDLE hThread;

void MyServiceContinue()
{
    logPrintf(LOG_DEBUG, "Try to continue service...\n");
    if (ResumeThread(hThread) >= 0)
    {
        logPrintf(LOG_INFORMATION, "Service continued.\n");
    }
    else
    {
        logPrintf(LOG_ERROR, "Error, service not continued!\n");
    }
}

void MyServicePause()
{
    logPrintf(LOG_DEBUG, "Try to pause service...\n");
    if (SuspendThread(hThread) >= 0)
    {
        logPrintf(LOG_INFORMATION, "Service paused.\n");
    }
    else
    {
        logPrintf(LOG_ERROR, "Error, service not continued!\n");
    }
}

void MyServiceStart()
{
    logPrintf(LOG_DEBUG, "Try to start service...\n");

    if (initDispatcher() == false)
    {
        logPrintf(LOG_ERROR, "initDispatcher failed!\n");
        PanicQuitService(ERROR_DISPATCHER_INIT);
        return;
    }

	hThread = CreateThread(NULL, 0, udp_server, NULL, 0, NULL);
	if (hThread == NULL)
	{
        logPrintf(LOG_ERROR, "CreateThread failed!\n");
        PanicQuitService(ERROR_THREAD_INIT);
        return;
	}

    logPrintf(LOG_INFORMATION, "Service is ready.\n");
}

void MyServiceStop()
{
    logPrintf(LOG_DEBUG, "Try to stop service...\n");
    // this will close pipe between service and child process
    // child process should terminate by it self (no more data on STDIN)
    cleanDispatcher();

    TerminateThread(hThread, 0);

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

DWORD WINAPI udp_server(LPVOID lpParam)
{
	WSADATA winsock;
	SOCKET server;
	SOCKADDR_IN sock_addr;
	char buffer[1024];
	int buffer_length;

	WSAStartup(MAKEWORD(2,2), &winsock);

	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(SERVER_LISTEN_PORT);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server, (SOCKADDR*)&sock_addr, sizeof(sock_addr)) != 0)
	{
        logPrintf(LOG_ERROR, "Bind error...\n");
	    return 1;
	}
    logPrintf(LOG_INFORMATION, "Listening on port %u\n", SERVER_LISTEN_PORT);

	do
	{
	    buffer_length = recv(server, buffer, sizeof(buffer), 0);
	    if (buffer_length != SOCKET_ERROR)
	    {
            logPrintf(LOG_DEBUG, "Data recieved from snmp stack\n");
            snmp_trap_decode(buffer, buffer_length);
	    }
	} while(buffer_length != SOCKET_ERROR);

	closesocket(server);

	WSACleanup();

	return 0;
}

int main( int argc, char *argv[])
{
    service myService;
	WSADATA winsock;

	WSAStartup(MAKEWORD(2,2), &winsock);

    myService.name = SERVICE_NAME;
    myService.description = SERVICE_DESC;
    //myService.dependencies = "SNMPTRAP\0""EventLog\0";
    myService.dependencies = NULL;
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
        logSetLevel(LOG_PEDANTIC);

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

    }
    else
    {
        logPrintf(LOG_INFORMATION, "Starting service...\n");
        logPrintf(LOG_DEBUG, "Windows service registering...\n");
        RunService();
        logPrintf(LOG_INFORMATION, "Service terminated.\n");
    }

    return EXIT_SUCCESS;
}
