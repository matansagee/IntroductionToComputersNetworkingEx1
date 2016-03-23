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

//***************************************************************
// Main Server - open file, open socket, accept conections and
//               and sending worker thread to each connection
//****************************************************************

void MainServer(int portNumberSender, int portNumberReceiver, double probability, int random_seed)
{
	//Sender
	SOCKADDR_IN serviceSender;
	//Receiver
	SOCKADDR_IN serviceReceiver;
	WSADATA wsaData;
	// Initialize Winsock.
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	MainSocketSender = INVALID_SOCKET;
	MainSocketReceiver = INVALID_SOCKET;

	if (StartupRes != NO_ERROR)
	{
		fprintf(stderr,"error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		return;
	}
	MainSocketSender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	MainSocketReceiver = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocketSender == INVALID_SOCKET)
	{
		fprintf(stderr,"Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}

	if (MainSocketReceiver == INVALID_SOCKET)
	{
		fprintf(stderr,"Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}
	//-------------------------------ESTABLISH CONNECTION---------------------------------
	establishConnection(&MainSocketSender, &serviceSender, portNumberSender);
	establishConnection(&MainSocketReceiver, &serviceReceiver, portNumberReceiver);
	
	
	SOCKET AcceptSocketSender;
	SOCKET AcceptSocketReceiver;
	AcceptSocketReceiver = accept(MainSocketReceiver, NULL, NULL);
	if (AcceptSocketReceiver == INVALID_SOCKET)
	{
		fprintf(stderr,"Accepting connection with client failed, error %ld\n", WSAGetLastError());
		exit(1);
	}
	fprintf(stderr,"receiver: %s\n", inet_ntoa(serviceReceiver.sin_addr));

	AcceptSocketSender = accept(MainSocketSender, NULL, NULL);
	if (AcceptSocketSender == INVALID_SOCKET)
	{
		fprintf(stderr,"Accepting connection with client failed, error %ld\n", WSAGetLastError());
		exit(1);
	}
	fprintf(stderr,"sender: %s\n", inet_ntoa(serviceSender.sin_addr));
	//-------------------------------END OF ESTABLISHING CONNECTION-------------------------
	//---------------------------------------GET DATA---------------------------------------
	TransferResult_t RecvRes;
	char *acceptedStr = NULL;
	//get string from sender
	RecvRes = ReceiveString(&acceptedStr, AcceptSocketSender);
	if (RecvRes == TRNS_FAILED)
	{
		fprintf(stderr,"Socket error while trying to write data to socket\n");
		exit(1);
	}
	//closing the sender socket to receive
	if (shutdown(AcceptSocketSender, SD_RECEIVE == SOCKET_ERROR)){
		fprintf(stderr,"Socket error while trying to shutdown socket\n");
		exit(1);
	}
	fprintf(stderr,"%d bytes ", strlen(acceptedStr) - ECC_BLOCK_LENGTH);

	//-------------------------FLIP BITS-------------------------------------
	int num_bits_flipped = 0;
	for (unsigned int i = 0; i < strlen(acceptedStr) - ECC_BLOCK_LENGTH; i++){
		for (unsigned int j = 0; j < 8; j++){
			if (probability > rand() * 2 + 2){//assuming RAND_MAX = 32767
				acceptedStr[i] ^= 1 << j;//toggle the bit
				num_bits_flipped++;
			}
			
		}
	}
	fprintf(stderr,"flipped %d bits\n", num_bits_flipped);
	//-----------------------------------------------------------------------
	//send string to receiver
	if (SendString(acceptedStr, AcceptSocketReceiver) == TRNS_FAILED)
	{
		fprintf(stderr,"Service socket error while writing, closing thread.\n");
	}

	//get string from receiver
	char* string_from_receiver = NULL;
	RecvRes = ReceiveString(&string_from_receiver, AcceptSocketReceiver);
	if (RecvRes == TRNS_FAILED)
	{
		fprintf(stderr,"Socket error while trying to write data to socket\n");
		exit(1);
	}
	//close receiver socket
	if (closesocket(AcceptSocketReceiver) == SOCKET_ERROR){
		fprintf(stderr,"Socket error while trying to close socket\n");
		exit(1);
	}
	//send string to sender
	if (SendString(string_from_receiver, AcceptSocketSender) == TRNS_FAILED)
	{
		fprintf(stderr,"Service socket error while writing, closing thread.\n");
	}
	//close sender socket
	if (closesocket(AcceptSocketSender) == SOCKET_ERROR){
		fprintf(stderr,"Socket error while trying to close socket\n");
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
		fprintf(stderr,"bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	int ListenRes;
	ListenRes = listen(*MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		fprintf(stderr,"Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}

}

