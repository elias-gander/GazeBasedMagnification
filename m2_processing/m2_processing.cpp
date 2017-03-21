// m2_processing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char *argv[])
{
	Producer trackingModule(argv[1], atoi(argv[2]));
	Consumer magnificationModule(argv[3], atoi(argv[4]));

	trackingModule.hookup();
	magnificationModule.hookup();

	bool okay = true;
	int x, y, freq;
	while(okay) {
		okay = trackingModule.take(&x, &y, &freq);
		// PROCESS TODO
		okay &= magnificationModule.give(x, y, -1);	// don't care about freq
	}

	wprintf(L"done...press enter to terminate");
	std::getchar();

	return 0;
}

