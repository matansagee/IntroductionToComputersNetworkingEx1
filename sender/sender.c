#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
//#pragma comment(lib, "IPHLPAPI.lib")

#include "SocketSendRecvTools.h"
#include "sender.h"
#include "code_functions.h"

SOCKET m_socket;

//********************************************************************
// MainClient - open a socket, ask for connection ,and manage data 
//              thransportation in from of the server by 2 threads
//********************************************************************
void MainClient(char* channelIp, FILE *file, int channelPort)
{
	SOCKADDR_IN clientService;
	struct sockaddr_in foo;
	int len = sizeof(struct sockaddr);
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		fprintf(stderr,"Error at WSAStartup()\n");

	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		fprintf(stderr,"Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(channelIp); //Setting the IP address to connect to
	clientService.sin_port = htons(channelPort); //Setting the port to connect to.
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
	{
		getsockname(m_socket, (struct sockaddr *) &foo, &len);
		WSACleanup();
		return;
	}

	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	char *acceptedStr = NULL;
	long input_file_size;
	char *fileContents;
	fseek(file, 0, SEEK_END);
	input_file_size = ftell(file);
	rewind(file);
	fileContents = malloc((input_file_size + 1) * (sizeof(char)));
	fread(fileContents, sizeof(char), input_file_size, file);
	fileContents[input_file_size] = '\0';
	fclose(file);
	input_file_size = strlen(fileContents);

	// Compute the ECC's
	uint16_t internet_checksum = checksum(fileContents, strlen(fileContents));
	uint16_t crc16code = gen_crc16(fileContents, strlen(fileContents));
	uint32_t crc32code = crc32a((char*)fileContents);

	char* coded_file_content = malloc((strlen(fileContents) + 17) * sizeof(char));
	if (coded_file_content == NULL)
	{
		fprintf(stderr,"Error - malloc\n");
		exit(1);
	}
	
	strcpy(coded_file_content, fileContents);
	char crc32char[32];
	char crc16char[32];
	char internet_checksum_char[32];

	_itoa(crc32code, crc32char, 16);
	_itoa(crc16code, crc16char, 16);
	_itoa(internet_checksum, internet_checksum_char, 16);
	strcat(coded_file_content, crc32char);
	strcat(coded_file_content, crc16char);
	strcat(coded_file_content, internet_checksum_char);
	coded_file_content[strlen(coded_file_content)] = '\0';

	SendRes = SendString(coded_file_content, m_socket);
	shutdown(m_socket, SD_SEND);

	if (SendRes == TRNS_FAILED)
	{
		fprintf(stderr,"Socket error while trying to write data to socket\n");
		exit(1);
	}

	RecvRes = ReceiveString(&acceptedStr, m_socket);
	if (RecvRes == TRNS_FAILED)
	{
		fprintf(stderr,"Socket error while trying to recv data to socket\n");
		exit(1);
	}

	fprintf(stderr,"%s\n", acceptedStr);
	free(acceptedStr);
	closesocket(m_socket);
	WSACleanup();
}

