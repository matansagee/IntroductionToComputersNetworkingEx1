#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"server.h"

//---------------------------------------------------------------------------
// Main.c -  includes of handling input parameters and file creations, 
//           calling main server prog
//---------------------------------------------------------------------------

int main(int argc, char** argv)
{
	int portNumberSender, portNumberReceiver;
	if (argc != 3)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	portNumberSender = atoi(argv[1]);
	portNumberReceiver = atoi(argv[2]);
	if (portNumberSender <= 0 && portNumberReceiver <= 0)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	MainServer(portNumberSender, portNumberReceiver);
}