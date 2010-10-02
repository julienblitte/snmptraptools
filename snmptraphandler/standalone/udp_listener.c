#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "winsock2.h"

// prototye de la procédure utilisée pour notre 2e thread
DWORD WINAPI udp_server(LPVOID lpParam);
HANDLE hThread;

int main(int argc, char* argv[])
{

	hThread = CreateThread(NULL, 0, udp_server, NULL, NULL, NULL);
	
	//WaitForSingleObject(hThread, INFINITE);	
	
	return 0;
}

// Procédure utilisée par le thread
DWORD WINAPI udp_server(LPVOID lpParam)
{
	SOCKET server;
	sockaddr_in sock_addr;
	char buffer[1024];
	int buffer_length;
	WSADATA winsock;

	sockaddr_in RecvAddr;

	WSAStartup(MAKEWORD(2,2), &winsock);

	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(162);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(server, (SOCKADDR*)&sock_addr, sizeof(sock_addr));

	buffer_length = sizeof(buffer);
	
	if (recvfrom(*((SOCKET *)lpParam), buffer, buffer_length, 0, NULL, NULL) == 0)
	{
	}

	closesocket(RecvSocket);

	WSACleanup();
	
	return 0;
}