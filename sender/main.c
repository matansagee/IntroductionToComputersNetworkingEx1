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
		fprintf(stderr,"ERROR - Not enough arguments\n");
		return 1;
	}
	channelPort = atoi(argv[2]);
	channelIp = (char*)malloc(strlen(argv[1])*sizeof(char));
	if (channelIp == NULL)
	{
		fprintf(stderr,"ERROR - Malloc failed \n");
		return 1;
	}
	strcpy(channelIp, argv[1]);
	fileName = (char*)malloc(strlen(argv[3])*sizeof(char));
	if (fileName == NULL)
	{
		fprintf(stderr,"ERROR - Malloc failed \n");
		return 1;
	}
	strcpy(fileName, argv[3]);
	FILE* file = fopen(fileName, "rb");
	if (file == NULL)
	{
		fprintf(stderr,"ERROR - open file %s failed\n",fileName);
		return 1;
	}

	MainClient(channelIp, file, channelPort);
	return 0;
}