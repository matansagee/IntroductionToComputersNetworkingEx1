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
#pragma comment(lib, "IPHLPAPI.lib")

#include "utils.h"
#include "SocketSendRecvTools.h"
#include "sender.h"

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
		RecvRes = ReceiveString( &acceptedStr , m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}
		else
		{
			fprintf(UsernameLogFile,"RECEIVED:: %s\n",acceptedStr);
			printf("%s\n",acceptedStr);
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
		
		SendRes = SendString( SendStr, m_socket);
		fprintf(UsernameLogFile,"SENT:: %s\n",SendStr);
		if ( SendRes == TRNS_FAILED ) 
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
void MainClient(char* channelIp,FILE *file,int channelPort)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	struct sockaddr_in foo;
	int len = sizeof(struct sockaddr);
    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }

    clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( channelIp ); //Setting the IP address to connect to
    clientService.sin_port = htons( channelPort ); //Setting the port to connect to.
	/*if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) 
	{
		getsockname(m_socket, (struct sockaddr *) &foo, &len);
        /*
		fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
			 inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
		fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
			 inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
			 *//*
		WSACleanup();
        return;
    }*/
	
	while (1)
	{
		TransferResult_t SendRes;
		TransferResult_t RecvRes;
	
		char SendStr[256];
		char *acceptedStr = NULL;
		long input_file_size;
		char *fileContents;
		unsigned char* file_content_coded;
		printf("sending to Receiver\n");
		fseek(file, 0, SEEK_END);
		input_file_size = ftell(file);
		rewind(file);
		fileContents = malloc(input_file_size * (sizeof(char)));
		fread(fileContents, sizeof(char), input_file_size, file);
		fileContents[input_file_size] = '\0';
		fclose(file);
		//----------------------------------------------CREATE BYTE ARRAY-------------------------
		FILE *fileptr;
		char *buffer;
		long filelen;

		fileptr = fopen("hello.txt", "rb");  // Open the file in binary mode
		fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
		filelen = ftell(fileptr);             // Get the current byte offset in the file
		rewind(fileptr);                      // Jump back to the beginning of the file

		buffer = (char *)malloc((filelen + 1)*sizeof(char)); // Enough memory for file + \0
		fread(buffer, filelen, 1, fileptr); // Read in the entire file
		fclose(fileptr); // Close the file
		//------------------------------------------------------------------------------------------------

		// Compute the 16-bit checksum
		//**********need to add a 0x00 byte at end if # of bytes isnt even
		unsigned short check = checksum(buffer, strlen(fileContents));

		// Output the checksum
		printf("checksum = %04X \n", check);

		uint16_t crc16code = gen_crc16(buffer, strlen(fileContents));
		printf("crc16 = %04X \n", crc16code);

		//uint32_t crc32code = crc32(0xFFFFFFFF,fileContents, strlen(fileContents));
		uint32_t crc32code = crc32a((char*) fileContents);

		printf("crc32 = %08X \n ", crc32code);

		SendRes = SendString("dsdsD", m_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}

		printf("waiting for message\n");
		RecvRes = ReceiveString(&acceptedStr, m_socket);
		printf("message - %s\n", acceptedStr);
		free(acceptedStr);
	}




	
	closesocket(m_socket);
	WSACleanup();
    
	return;
}
char* calc_crc32(char* fileContents)
{
	return "a";
}

char* calc_crc16(char* fileContents)
{
	return "a";
}
char* calc_internet_checksum(char* fileContents)
{
	return "ds";
}

/*
**************************************************************************
Function: tcp_sum_calc()
**************************************************************************
Description:
Calculate TCP checksum
***************************************************************************
Copy from http://www.netfor2.com/tcpsum.htm
*/


//word16 tcp_sum_calc(word16 len_tcp, BOOL padding, word16 buff[])
//{
//	word16 prot_tcp = 6;
//	word16 padd = 0;
//	word16 word16;
//	long sum;
//	int i;
//
//	// Find out if the length of data is even or odd number. If odd,
//	// add a padding byte = 0 at the end of packet
//	if (padding & 1 == 1){
//		padd = 1;
//		buff[len_tcp] = 0;
//	}
//
//	//initialize sum to zero
//	sum = 0;
//
//	// make 16 bit words out of every two adjacent 8 bit words and 
//	// calculate the sum of all 16 vit words
//	for (i = 0; i<len_tcp + padd; i = i + 2){
//		word16 = ((buff[i] << 8) & 0xFF00) + (buff[i + 1] & 0xFF);
//		sum = sum + (unsigned long)word16;
//	}
//	
//	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
//	while (sum >> 16)
//		sum = (sum & 0xFFFF) + (sum >> 16);
//
//	// Take the one's complement of sum
//	sum = ~sum;
//
//	return ((unsigned short)sum);
//}
//
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