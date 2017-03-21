#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Consumer.h"

#define BUFFER_LEN 512

Consumer::Consumer() {}
// empty constructor when construction parameters are not known at object creation time -> use init functions
void Consumer::init(char* path) {
	isFile = true;

	this->path = path;

	bufferPos = 0;
}
void Consumer::init(char* ip, int port) {
	isFile = false;

	this->ip = ip;
	this->port = port;

	bufferPos = 0;
}

Consumer::Consumer(char* path) {
	init(path);
}

Consumer::Consumer(char* ip, int port) {
	init(ip, port);
}

int Consumer::hookup() {
	if (isFile) {
		file.open(path);
		if (!file.is_open()) {
			printf("Failed to open output file %s\n", path);
			return 1;
		}
		printf("Sucessfully opened output file %s\n", path);
	}
	else {
		// taken from https://msdn.microsoft.com/en-us/library/windows/desktop/ms737526(v=vs.85).aspx
		//----------------------
		// Initialize Winsock.
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR) {
			wprintf(L"WSAStartup failed with error: %ld\n", iResult);
			return iResult;
		}
		//----------------------
		// Create a SOCKET for listening for
		// incoming connection requests.
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listenSocket == INVALID_SOCKET) {
			wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return WSAGetLastError();
		}
		//----------------------
		// The sockaddr_in structure specifies the address family,
		// IP address, and port for the socket that is being bound.
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr(ip);	// any?
		service.sin_port = htons(port);

		if (bind(listenSocket,
			(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
			wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return WSAGetLastError();
		}
		//----------------------
		// Listen for incoming connection requests.
		// on the created socket
		if (listen(listenSocket, 1) == SOCKET_ERROR) {
			wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return WSAGetLastError();
		}

		wprintf(L"Waiting for client to connect...\n");

		//----------------------
		// Create a SOCKET for accepting incoming requests.

		//----------------------
		// Accept the connection.
		acceptSocket = accept(listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return WSAGetLastError();
		}

		wprintf(L"Client connected.\n");
	}
	return 0;
}

bool Consumer::give(int x, int y, int freq) {
	std::stringstream tripleStream;
	tripleStream << x << ',' << y << ',' << freq << '\n';
	std::string triple = tripleStream.str();
	if (bufferPos + 1 + triple.length() >= BUFFER_LEN) {
		// time to empty the buffer
		if (!emptyBuffer()) {
			return false;
		}
	}
	bufferPos += triple.length();
	bufferStream << triple;
	return !bufferStream.fail() && !bufferStream.bad();
}

bool Consumer::emptyBuffer() {
	if (isFile) {
		// write to file
		file << bufferStream.rdbuf();
		if (file.fail() || bufferStream.fail()) {
			wprintf(L"Error writing to output file!\n");
			return false;
		}
		// reset buffer
		bufferStream.str();
		bufferStream.clear();
		bufferPos = 0;
	}
	else {
		std::string bufferString = bufferStream.str();

		int bytesSent = send(acceptSocket, bufferString.c_str(), bufferPos, 0);
		if (bytesSent == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(acceptSocket);

			WSACleanup();
			return false;
		}
		printf("Bytes Sent: %ld\n", bytesSent);

		// reset buffer stream
		bufferPos = 0;
		bufferStream.str("");
		bufferStream.clear();
	}

	return true;
}