#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include"server.h"

//---------------------------------------------------------------------------
// Main.c -  includes of handling input parameters and file creations, 
//           calling main server prog
//---------------------------------------------------------------------------

int main(int argc, char** argv)
{
	int portNumberSender, portNumberReceiver;
	if (argc != 5)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	portNumberSender = atoi(argv[1]);
	portNumberReceiver = atoi(argv[2]);
	//double probability = (atoi(argv[3]) / pow(2, 16));
	double probability = atoi(argv[3]);
	int random_seed = atoi(argv[4]);
	if (portNumberSender <= 0 && portNumberReceiver <= 0)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	MainServer(portNumberSender, portNumberReceiver, probability, random_seed);
}