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

#include "utils.h"
#include "SocketSendRecvTools.h"
#include "sender.h"
#include "crc32.h"

SOCKET m_socket;
FILE *UsernameErrorsFile;
FILE *UsernameLogFile;

char* calc_crc32(char* fileContents);
char* calc_crc16(char* fileContents);
char* calc_internet_checksum(char* fileContents);

//----- Type defines ----------------------------------------------------------
typedef unsigned char      byte;    // Byte is a char
typedef unsigned short int word16;  // 16-bit word is a short int
typedef unsigned int       word32;  // 32-bit word is an int

word16 checksum(byte *addr, word32 count);
unsigned short crc16(char *data_p, unsigned short length);

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
void MainClient(char* channelIp, FILE *file, int channelPort)
{
	SOCKADDR_IN clientService;
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

	// Compute the 16-bit checksum
	uint16_t internet_checksum = checksum(fileContents, strlen(fileContents));
	uint16_t crc16code = gen_crc16(fileContents, strlen(fileContents));
	uint32_t crc32code = crc32a((char*)fileContents);

	char* coded_file_content = malloc((strlen(fileContents) + 17) * sizeof(char));
	if (coded_file_content == NULL)
	{
		printf("Error - malloc\n");
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
		printf("Socket error while trying to write data to socket\n");
		exit(1);
	}

	RecvRes = ReceiveString(&acceptedStr, m_socket);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Socket error while trying to recv data to socket\n");
		exit(1);
	}

	printf("%s\n", acceptedStr);
	free(acceptedStr);
	closesocket(m_socket);
	WSACleanup();
}

//=============================================================================
//=  Compute Internet Checksum for count bytes beginning at location addr     =
//=============================================================================
word16 checksum(byte *addr, word32 count)
{
	register word32 sum = 0;
	//check if count is even, if not add 0x00
	if (count % 2 != 0){

	}
	// Main summing loop
	while (count > 1)
	{
		sum = sum + *((word16 *)addr)++;
		count = count - 2;
	}

	// Add left-over byte, if any
	if (count > 0)
		sum = sum + *((byte *)addr);

	// Fold 32-bit sum to 16 bits
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return(~sum);
}

#define POLY 0x8408
/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

unsigned short crc16(char *data_p, unsigned short length)
{
	unsigned char i;
	unsigned int data;
	unsigned int crc = 0xffff;

	if (length == 0)
		return (~crc);

	do
	{
		for (i = 0, data = (unsigned int)0xff & *data_p++;
			i < 8;
			i++, data >>= 1)
		{
			if ((crc & 0x0001) ^ (data & 0x0001))
				crc = (crc >> 1) ^ POLY;
			else  crc >>= 1;
		}
	} while (--length);

	crc = ~crc;
	data = crc;
	crc = (crc << 8) | (data >> 8 & 0xff);

	return (crc);
}