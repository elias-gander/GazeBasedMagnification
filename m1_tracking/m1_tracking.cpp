// m1_tracking.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, char* argv[])
{
	Producer dataSource;
	Consumer dataSink;

	if (argc > 2 && strcmp(argv[1], "--input-file") == 0) {
		dataSource.init(argv[2]);
	}
	else {
		// TODO parse tracking data from eyeX
	}
	dataSource.hookup();

	if (argc > 4 && strcmp(argv[3], "--output-file") == 0) {
		dataSink.init(argv[4]);
	}
	else if(argc > 4) {
		dataSink.init(argv[3], atoi(argv[4]));
	}
	else {
		dataSink.init(argv[1], atoi(argv[2]));
	}
	dataSink.hookup();

	bool okay = true;
	int x, y, freq;
	while(okay) {
		okay = dataSource.take(&x, &y, &freq);
		okay &= dataSink.give(x, y, freq);
	}

	wprintf(L"done...press enter to terminate");
	std::getchar();

	return 0;
}