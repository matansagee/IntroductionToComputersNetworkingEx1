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

#include "SocketSendRecvTools.h"
#include "client.h"
#include "code_functions.h"

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

void 	write_content_message_to_file(char* fileName, char* message_content)
{
	FILE* output_file = fopen(fileName, "w+");
	fprintf(output_file, message_content);
	fclose(output_file);
}

char* get_submit_string(int received_bytes,BOOL crc32_pass, BOOL crc16_pass,BOOL internet_checksum_pass)
{
	char temp_string[80];
	char* send_str = malloc(256 * sizeof(char));
	if (send_str == NULL)
	{
		return send_str;
	}
	memset(send_str, '\0', sizeof(send_str));
	strcpy(send_str, "recieved: ");
	strcat(send_str, _itoa(received_bytes,temp_string,10));
	strcat(send_str, " bytes \n");
	strcat(send_str, "CRC-32: ");
	if (crc32_pass)	{
		strcat(send_str, "pass; ");
	}
	else	{
		strcat(send_str, "FAIL; ");
	}
	strcat(send_str, "CRC-16: ");
	if (crc16_pass)	{
		strcat(send_str, "pass; ");
	}
	else	{
		strcat(send_str, "FAIL; ");
	}
	strcat(send_str, "Internet cksum: ");
	if (internet_checksum_pass)	{
		strcat(send_str, "pass");
	}
	else	{
		strcat(send_str, "FAIL");
	}

	return send_str;

}

void MainClient(char* channelIp,char* fileName,int channelPort)
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

	TransferResult_t RecvRes;
	TransferResult_t SendRes;
	char *acceptedStr = NULL;


	RecvRes = ReceiveString(&acceptedStr, m_socket);

	char* message_content = malloc((strlen(acceptedStr) - 8 * 2) * sizeof(char));
	if (message_content == NULL)
	{
		printf("Error - malloc");
		exit(1);
	}
	strncpy(message_content, acceptedStr, strlen(acceptedStr) - 9);
	message_content[strlen(acceptedStr) - 16] = '\0';
	
	write_content_message_to_file(fileName, message_content);

	char* crc32code = malloc(4 * sizeof(char));
	if (crc32code == NULL)
	{
		printf("Error - malloc");
		exit(1);
	}
	strncpy(crc32code, acceptedStr + strlen(acceptedStr) - 16, 8);
	crc32code[8] = '\0';

	char* crc16code = malloc(2 * sizeof(char));
	if (crc16code == NULL)
	{
		printf("Error - malloc");
		exit(1);
	}
	strncpy(crc16code, acceptedStr + strlen(acceptedStr) - 8, 4);
	crc16code[4] = '\0';

	char* internet_checksum = malloc(2 * sizeof(char));
	if (internet_checksum == NULL)
	{
		printf("Error - malloc");
		exit(1);
	}
	strncpy(internet_checksum, acceptedStr + strlen(acceptedStr) - 4, 4);
	internet_checksum[4] = '\0';


	//------------------------------CALCULATE CODES AND COMPARE---------------------------
	uint32_t crc32code_calc = crc32a(message_content);
	uint16_t crc16code_calc = gen_crc16(message_content, strlen(message_content));
	uint16_t internet_checksum_calc = checksum(message_content, strlen(message_content));
	BOOL crc32_pass, crc16_pass, internet_checksum_pass;
	
	char temp[32];
	int received_bytes = strlen(acceptedStr);
	printf("received: %d bytes written: %d bytes\n", received_bytes, strlen(message_content)*sizeof(char));
	//--------------------------------------CRC32-------------------------------
	printf("CRC-32: ");
	if (crc32code_calc != (uint32_t)strtoul(crc32code, &temp, 16))	{
		printf("FAIL. "); 
		crc32_pass = FALSE;
	}	else 	{
		printf("pass. ");
		crc32_pass = TRUE;
	}
	printf("Computed 0x%04x, received 0x%s\n", crc32code_calc, crc32code);
	//--------------------------------------CRC16-------------------------------
	printf("CRC-16: ");
	if (crc16code_calc != (uint16_t)strtoul(crc16code, &temp, 16))	{
		printf("FAIL. ");
		crc16_pass = FALSE;
	}	else	{
		printf("pass. ");
		crc16_pass = TRUE;
	}
	printf("Computed 0x%04x, received 0x%s\n", crc16code_calc, crc16code);
	//---------------------------------INTERNET CHECKSUM------------------------
	printf("Inet-cksum: ");
	if (internet_checksum_calc != strtoul(internet_checksum, &temp, 16))	{
		printf("FAIL. ");
		internet_checksum_pass = FALSE;
	}	else	{
		printf("pass. ");
		internet_checksum_pass = TRUE;
	}
	printf("Computed 0x%04x, received 0x%s\n", internet_checksum_calc, internet_checksum);
	//-----------------------------------------------------------------------------------

	shutdown(m_socket, SD_RECEIVE);
	
	char* send_str = get_submit_string(received_bytes, crc32_pass, crc16_pass, internet_checksum_pass);
	if (send_str == NULL)
	{
		printf("Error - malloc failed\n");
		exit(1);
	}
	//printf("%s\n", send_str);

	SendRes = SendString(send_str, m_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		exit(1);
	}
	
	
	free(acceptedStr);

	closesocket(m_socket);
	WSACleanup();
	return;
}

