#pragma once
#include <WinSock2.h>
#include <sstream>
#include <fstream>

class Consumer {
public:
	Consumer();
	void init(char* path);
	void init(char* ip, int port);
	Consumer(char* path);
	Consumer(char* ip, int port);
	int hookup();
	bool give(int x, int y, int freq);

private:
	bool isFile;
	// connection info
	char* ip;
	int port;
	SOCKET listenSocket;
	SOCKET acceptSocket;
	// file info
	char* path;
	std::ofstream file;
	// data
	std::stringstream bufferStream;
	int bufferPos;

	bool emptyBuffer();
};