#pragma once
#include <WinSock2.h>
#include <sstream>
#include <fstream>

class Producer {
public:
	Producer();
	void init(char* path);
	void init(char* ip, int port);
	Producer(char* path);
	Producer(char* ip, int port);
	int hookup();
	bool take(int* x, int* y, int* freq);
	~Producer();

private:
	bool isFile;
	// connection info
	char* ip;
	int port;
	SOCKET connectSocket;
	// file info
	char* path;
	std::ifstream file;
	// data
	std::stringstream bufferStream;
	char* rest;

	bool fillBuffer();
	
};

