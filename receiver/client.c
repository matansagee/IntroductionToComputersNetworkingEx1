#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include "utils.h"
#include "SocketSendRecvTools.h"
#include "client.h"

SOCKET m_socket;
FILE *UsernameErrorsFile;
FILE *UsernameLogFile;


//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;

	while (1)
	{
		char *acceptedStr = NULL;
		RecvRes = ReceiveString(&acceptedStr, m_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}
		else
		{
			fprintf(UsernameLogFile, "RECEIVED:: %s\n", acceptedStr);
			printf("%s\n", acceptedStr);
		}

		free(acceptedStr);
	}

	return 0;
}

//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;

	while (1)
	{
		gets(SendStr); //Reading a string from the keyboard

		SendRes = SendString(SendStr, m_socket);
		fprintf(UsernameLogFile, "SENT:: %s\n", SendStr);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
	}
}

//********************************************************************
// MainClient - open a socket, ask for connection ,and manage data 
//              thransportation in from of the server by 2 threads
//********************************************************************
void MainClient(char* serverIp, char* clientName, int serverPort)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	struct sockaddr_in foo;
	int len = sizeof(struct sockaddr);
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");

	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(serverIp); //Setting the IP address to connect to
	clientService.sin_port = htons(serverPort); //Setting the port to connect to.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
	{
		getsockname(m_socket, (struct sockaddr *) &foo, &len);
		/*
		fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
		inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
		fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
		inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
		*/
		WSACleanup();
		return;
	}
	
	while (1)
	{

		TransferResult_t RecvRes;
		char SendStr[256];
		TransferResult_t SendRes;

		char *acceptedStr = NULL;
		printf("waiting for message\n");
		RecvRes = ReceiveString(&acceptedStr, m_socket);
		printf("message - %s\n", acceptedStr);
		printf("sending to Sender\n");

		gets(SendStr); //Reading a string from the keyboard
		SendRes = SendString(SendStr, m_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		free(acceptedStr);
	}
	
	closesocket(m_socket);
	WSACleanup();

	return;
}

