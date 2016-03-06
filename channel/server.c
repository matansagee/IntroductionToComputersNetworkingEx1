#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

#include "server.h"
#include "utils.h"
#include "SocketSendRecvTools.h"

#define SEND_STR_SIZE 35
FILE *ServerLog;

SOCKET MainSocketSender;
SOCKET MainSocketReceiver;


//***************************************************************
// Main Server - open file, open socket, accept conections and
//               and sending worker thread to each connection
//****************************************************************

void MainServer(int portNumberSender, int portNumberReceiver)
{
	int Ind;
	int Loop;
	//Sender
	SOCKADDR_IN serviceSender;
	int bindResSender;
	int ListenResSender;
	//Receiver
	SOCKADDR_IN serviceReceiver;
	int bindResReceiver;
	int ListenResReceiver;
	
	WSADATA wsaData;
	DWORD waitRes;
	// Initialize Winsock.
	int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           
	MainSocketSender = INVALID_SOCKET;
	MainSocketReceiver = INVALID_SOCKET;

	if ( StartupRes != NO_ERROR )
	{
		printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		return;
	}
	MainSocketSender = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	MainSocketReceiver = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if ( MainSocketSender == INVALID_SOCKET ) 
	{
		printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
		exit(1);
	}

	if (MainSocketReceiver == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}

	serviceSender.sin_family = AF_INET;
	serviceSender.sin_addr.s_addr = INADDR_ANY;
	serviceSender.sin_port = htons(portNumberSender); //The htons function converts a u_short from host to TCP/IP network byte order 
	
	serviceReceiver.sin_family = AF_INET;
	serviceReceiver.sin_addr.s_addr = INADDR_ANY;
	serviceReceiver.sin_port = htons(portNumberReceiver); //The htons function converts a u_short from host to TCP/IP network byte order 


	bindResSender = bind(MainSocketSender, (SOCKADDR*)&serviceSender, sizeof(serviceSender));
	bindResReceiver = bind(MainSocketReceiver, (SOCKADDR*)&serviceReceiver, sizeof(serviceReceiver));

	if ( bindResSender == SOCKET_ERROR ) 
	{
		printf( "bind( ) failed with error %ld. Ending program\n", WSAGetLastError( ) );
		exit(1);
	}

	if (bindResReceiver == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	ListenResSender = listen( MainSocketSender, SOMAXCONN );
	ListenResReceiver = listen(MainSocketReceiver, SOMAXCONN);

	if ( ListenResSender == SOCKET_ERROR ) 
	{
		printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		exit(1);
	}

	if (ListenResReceiver == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}

	SOCKET AcceptSocketSender;
	SOCKET AcceptSocketReceiver;
	printf("waiting for Receiver to connect\n");
	AcceptSocketReceiver = accept( MainSocketReceiver, NULL, NULL );
	if ( AcceptSocketReceiver == INVALID_SOCKET )
	{
		printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
		exit(1);
	}
	printf("Receiver connected\n");
	printf("waiting for Sender to connect\n");
	AcceptSocketSender = accept(MainSocketSender, NULL, NULL);
	if (AcceptSocketSender == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		exit(1);
	}
	printf("Sender connected\n");

	while (1)
	{
		TransferResult_t RecvRes;
		char *acceptedStr = NULL;
		printf("waiting for message from Sender\n");
		RecvRes = ReceiveString(&acceptedStr, AcceptSocketSender);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		printf("Sending message to Receiver\n");
		if (SendString(acceptedStr, AcceptSocketReceiver) == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
		}
		printf("waiting for message from Receiver\n");
		free(acceptedStr);
		acceptedStr = NULL;
		RecvRes = ReceiveString(&acceptedStr, AcceptSocketReceiver);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		printf("Sending message to Sender\n");
		if (SendString(acceptedStr, AcceptSocketSender) == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
		}
	}


	
}

//*****************************************************
// CleanupWorkerThreads - check recv string from client
//*****************************************************
int ValidateReceivingString(TransferResult_t recvRes)
{
	if ( recvRes == TRNS_FAILED )
	{
		printf( "Service socket error while reading, closing thread.\n" );
		return 0;
	}
	else if ( recvRes == TRNS_DISCONNECTED )
	{
		printf( "Connection closed while reading, closing thread.\n" );
		return 0;
	}
	return 1;
}

//*******************************************************
// CloseSession - last thread will close the main socket
//                and close server file
//*******************************************************
void CloseSession(SOCKET sourceSocket)
{
}
