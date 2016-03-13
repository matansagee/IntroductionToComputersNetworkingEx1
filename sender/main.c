#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include"SocketSendRecvTools.h"
#include"sender.h"

int main(int argc, char** argv)
{
	char* channelIp;
	int channelPort;
	char* fileName;

	if (argc < 4)
	{
		printf("ERROR - Not enough arguments\n");
		return 1;
	}
	channelPort = atoi(argv[2]);
	channelIp = (char*)malloc(strlen(argv[1])*sizeof(char));
	if (channelIp == NULL)
	{
		printf("ERROR - Malloc failed \n");
		return 1;
	}
	strcpy(channelIp, argv[1]);
	fileName = (char*)malloc(strlen(argv[3])*sizeof(char));
	if (fileName == NULL)
	{
		printf("ERROR - Malloc failed \n");
		return 1;
	}
	strcpy(fileName, argv[3]);
	FILE* file = fopen(fileName, "rb");
	if (file == NULL)
	{
		printf("ERROR - open file %s failed\n",fileName);
		return 1;
	}
	//unsigned char buffer[10];
	//fread(buffer, sizeof(buffer), 1, file); // read 10 bytes to our buffer

	MainClient(channelIp, file, channelPort);
	return 0;
}