#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

#include "server.h"
#include "SocketSendRecvTools.h"

#define SEND_STR_SIZE 35
#define ECC_BLOCK_LENGTH 16
FILE *ServerLog;

SOCKET MainSocketSender;
SOCKET MainSocketReceiver;

void establishConnection(SOCKET *MainSocket, SOCKADDR_IN *service, int portNumber);
void PrintBitsOfChar(char c);

//***************************************************************
// Main Server - open file, open socket, accept conections and
//               and sending worker thread to each connection
//****************************************************************

void MainServer(int portNumberSender, int portNumberReceiver, double probability, int random_seed)
{
	//Sender
	SOCKADDR_IN serviceSender;
	int bindResSender;
	int ListenResSender;
	//Receiver
	SOCKADDR_IN serviceReceiver;
	int bindResReceiver;
	int ListenResReceiver;

	WSADATA wsaData;
	// Initialize Winsock.
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	MainSocketSender = INVALID_SOCKET;
	MainSocketReceiver = INVALID_SOCKET;

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		return;
	}
	MainSocketSender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	MainSocketReceiver = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocketSender == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}

	if (MainSocketReceiver == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}
	//-------------------------------ESTABLISH CONNECTION---------------------------------
	establishConnection(&MainSocketSender, &serviceSender, portNumberSender);
	establishConnection(&MainSocketReceiver, &serviceReceiver, portNumberReceiver);
	/*
	serviceSender.sin_family = AF_INET;
	serviceSender.sin_addr.s_addr = INADDR_ANY;
	serviceSender.sin_port = htons(portNumberSender); //The htons function converts a u_short from host to TCP/IP network byte order 

	serviceReceiver.sin_family = AF_INET;
	serviceReceiver.sin_addr.s_addr = INADDR_ANY;
	serviceReceiver.sin_port = htons(portNumberReceiver); //The htons function converts a u_short from host to TCP/IP network byte order 


	bindResSender = bind(MainSocketSender, (SOCKADDR*)&serviceSender, sizeof(serviceSender));
	bindResReceiver = bind(MainSocketReceiver, (SOCKADDR*)&serviceReceiver, sizeof(serviceReceiver));

	if (bindResSender == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	if (bindResReceiver == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	ListenResSender = listen(MainSocketSender, SOMAXCONN);
	ListenResReceiver = listen(MainSocketReceiver, SOMAXCONN);

	if (ListenResSender == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}

	if (ListenResReceiver == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}
	*/

	SOCKET AcceptSocketSender;
	SOCKET AcceptSocketReceiver;
	AcceptSocketReceiver = accept(MainSocketReceiver, NULL, NULL);
	if (AcceptSocketReceiver == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		exit(1);
	}
	printf("receiver: %s\n", inet_ntoa(serviceReceiver.sin_addr));

	AcceptSocketSender = accept(MainSocketSender, NULL, NULL);
	if (AcceptSocketSender == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		exit(1);
	}
	printf("sender: %s\n", inet_ntoa(serviceSender.sin_addr));
	//-------------------------------END OF ESTABLISHING CONNECTION-------------------------
	//---------------------------------------GET DATA---------------------------------------
	TransferResult_t RecvRes;
	char *acceptedStr = NULL;
	//get string from sender
	RecvRes = ReceiveString(&acceptedStr, AcceptSocketSender);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		exit(1);
	}
	//closing the sender socket to receive
	if (shutdown(AcceptSocketSender, SD_RECEIVE == SOCKET_ERROR)){
		printf("Socket error while trying to shutdown socket\n");
		exit(1);
	}
	printf("%d bytes ", strlen(acceptedStr) - ECC_BLOCK_LENGTH);

	//-------------------------FLIP BITS-------------------------------------
	int num_bits_flipped = 0;
	for (int i = 0; i < strlen(acceptedStr) - ECC_BLOCK_LENGTH; i++){
		for (int j = 0; j < 8; j++){
			if (probability == rand() * 2 + 2){//assuming RAND_MAX = 32767
				acceptedStr[i] ^= 1 << j;//toggle the bit
				num_bits_flipped++;
			}
			
		}
	}
	printf("flipped %d bits\n", num_bits_flipped);
	//-----------------------------------------------------------------------
	//send string to receiver
	if (SendString(acceptedStr, AcceptSocketReceiver) == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
	}

	//get string from receiver
	char* string_from_receiver = NULL;
	RecvRes = ReceiveString(&string_from_receiver, AcceptSocketReceiver);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		exit(1);
	}
	//close receiver socket
	if (closesocket(AcceptSocketReceiver) == SOCKET_ERROR){
		printf("Socket error while trying to close socket\n");
		exit(1);
	}
	//send string to sender
	if (SendString(string_from_receiver, AcceptSocketSender) == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
	}
	//close sender socket
	if (closesocket(AcceptSocketSender) == SOCKET_ERROR){
		printf("Socket error while trying to close socket\n");
		exit(1);
	}


	free(acceptedStr);
	free(string_from_receiver);
	WSACleanup();
}

void establishConnection(SOCKET *MainSocket, SOCKADDR_IN *service, int portNumber){
	service->sin_family = AF_INET;
	service->sin_addr.s_addr = INADDR_ANY;
	service->sin_port = htons(portNumber); //The htons function converts a u_short from host to TCP/IP network byte order 

	int bindRes;
	bindRes = bind(*MainSocket, (SOCKADDR*)service, sizeof(*service));

	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	int ListenRes;
	ListenRes = listen(*MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}

}

void PrintBitsOfChar(char c){
	for (int i = 7; i >= 0; i--){
		char bit = (c >> i) & 1;
		printf("%d", bit);
	}
	printf("\n");
}

//*****************************************************
// CleanupWorkerThreads - check recv string from client
//*****************************************************
int ValidateReceivingString(TransferResult_t recvRes)
{
	if (recvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		return 0;
	}
	else if (recvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		return 0;
	}
	return 1;
}

