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

void write_content_message_to_file(char* fileName, char* message_content)
{
	FILE* output_file = fopen(fileName, "w+");
	if (output_file == NULL)
	{
		fprintf(stderr, "ERROR - open out put file %s", fileName);
		exit(1);
	}
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
	if (iResult != NO_ERROR){
		fprintf(stderr,"Error at WSAStartup()\n");
	}
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

	TransferResult_t RecvRes;
	TransferResult_t SendRes;
	char *acceptedStr = NULL;


	RecvRes = ReceiveString(&acceptedStr, m_socket);
	//get the message content
	char* message_content = malloc(strlen(acceptedStr)*sizeof(char) - 8 + 1);
	if (message_content == NULL)
	{
		fprintf(stderr,"Error - malloc");
		exit(1);
	}
	strncpy(message_content, acceptedStr, strlen(acceptedStr) - 8);
	message_content[strlen(acceptedStr) - 8] = '\0';
	
	write_content_message_to_file(fileName, message_content);
	//get the crc32 code
	char* crc32code = malloc(5 * sizeof(char));
	if (crc32code == NULL)
	{
		fprintf(stderr,"Error - malloc");
		exit(1);
	}
	strncpy(crc32code, acceptedStr + strlen(acceptedStr) - 8, 4);
	crc32code[4] = '\0';
	//get the crc16code
	char* crc16code = malloc(3 * sizeof(char));
	if (crc16code == NULL)
	{
		fprintf(stderr,"Error - malloc");
		exit(1);
	}
	strncpy(crc16code, acceptedStr + strlen(acceptedStr) - 4, 2);
	crc16code[2] = '\0';
	//get the internet checksum code
	char* internet_checksum = malloc(3 * sizeof(char));
	if (internet_checksum == NULL)
	{
		fprintf(stderr,"Error - malloc");
		exit(1);
	}
	strncpy(internet_checksum, acceptedStr + strlen(acceptedStr) - 2, 2);
	internet_checksum[2] = '\0';
	//convert the codes to uint16_t and uint32_t
	uint16_t crc16code_int = (uint16_t)(crc16code[1]) << 8 | (uint16_t)crc16code[0] & 0xff;
	uint16_t internet_checksum_int = (uint16_t)(internet_checksum[1]) << 8 | ((uint16_t)internet_checksum[0] & 0xff);
	uint32_t crc32code_int = ((uint32_t)(crc32code[3]) << 24) | ((uint32_t)(crc32code[2]) << 16 & 0xffffffff)
		| ((uint32_t)(crc32code[1]) << 8 & 0xffff) | ((uint32_t)crc32code[0] & 0xff);
	//------------------------------CALCULATE CODES AND COMPARE---------------------------
	uint32_t crc32code_calc = crc32a(message_content);
	uint16_t crc16code_calc = gen_crc16(message_content, strlen(message_content));
	uint16_t internet_checksum_calc = checksum(message_content, strlen(message_content));
	BOOL crc32_pass, crc16_pass, internet_checksum_pass;
	
	char* temp;
	int received_bytes = strlen(acceptedStr);
	fprintf(stderr,"received: %d bytes written: %d bytes\n", received_bytes, strlen(message_content)*sizeof(char));
	//--------------------------------------CRC32-------------------------------
	fprintf(stderr,"CRC-32: ");
	if (crc32code_calc != crc32code_int)	{
		fprintf(stderr,"FAIL. "); 
		crc32_pass = FALSE;
	}	else 	{
		fprintf(stderr, "pass. ");
		crc32_pass = TRUE;
	}
	fprintf(stderr,"Computed 0x%04x, received 0x%0x\n", crc32code_calc, crc32code_int);
	//--------------------------------------CRC16-------------------------------
	fprintf(stderr,"CRC-16: ");
	if (crc16code_calc != crc16code_int)	{
		fprintf(stderr,"FAIL. ");
		crc16_pass = FALSE;
	}	else	{
		fprintf(stderr,"pass. ");
		crc16_pass = TRUE;
	}
	fprintf(stderr,"Computed 0x%04x, received 0x%0x\n", crc16code_calc, crc16code_int);
	//---------------------------------INTERNET CHECKSUM------------------------
	fprintf(stderr,"Inet-cksum: ");
	if (internet_checksum_calc != internet_checksum_int)	{
		fprintf(stderr,"FAIL. ");
		internet_checksum_pass = FALSE;
	}	else	{
		fprintf(stderr,"pass. ");
		internet_checksum_pass = TRUE;
	}
	fprintf(stderr,"Computed 0x%04x, received 0x%0x\n", internet_checksum_calc, internet_checksum_int);
	//-----------------------------------------------------------------------------------

	shutdown(m_socket, SD_RECEIVE);
	
	char* send_str = get_submit_string(received_bytes, crc32_pass, crc16_pass, internet_checksum_pass);
	if (send_str == NULL)
	{
		fprintf(stderr,"Error - malloc failed\n");
		exit(1);
	}

	SendRes = SendString(send_str, m_socket);

	if (SendRes == TRNS_FAILED)
	{
		fprintf(stderr,"Socket error while trying to write data to socket\n");
		exit(1);
	}
	
	
	free(acceptedStr);

	closesocket(m_socket);
	WSACleanup();
	return;
}

