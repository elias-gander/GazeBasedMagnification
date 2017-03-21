#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Producer.h"

#define BUFFER_LEN 512

Producer::Producer() {}
// empty constructor when construction parameters are not known at object creation time -> use init functions
void Producer::init(char* path) {
	isFile = true;

	rest = nullptr;

	this->path = path;
}
void Producer::init(char* ip, int port) {
	isFile = false;

	rest = nullptr;

	this->ip = ip;
	this->port = port;
}

Producer::Producer(char* path) {
	init(path);
}

Producer::Producer(char* ip, int port) {
	init(ip, port);
}

int Producer::hookup() {
	if (isFile) {
		file.open(path);
		if (!file.is_open()) {
			printf("Failed to open input file %s\n", path);
			return 1;
		}
		printf("Sucessfully opened input file %s\n", path);
	}
	else {
		// taken from https://msdn.microsoft.com/en-us/library/windows/desktop/ms737625(v=vs.85).aspx
		//----------------------
		// Initialize Winsock
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR) {
			wprintf(L"WSAStartup function failed with error: %d\n", iResult);
			return iResult;
		}
		//----------------------
		// Create a SOCKET for connecting to server
		connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connectSocket == INVALID_SOCKET) {
			wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return WSAGetLastError();
		}

		//----------------------
		// The sockaddr_in structure specifies the address family,
		// IP address, and port of the server to be connected to.
		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(ip);
		clientService.sin_port = htons(port);

		//----------------------
		// Connect to server.
		iResult = connect(connectSocket, (SOCKADDR *)& clientService, sizeof(clientService));
		if (iResult == SOCKET_ERROR) {
			wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
			iResult = closesocket(connectSocket);
			if (iResult == SOCKET_ERROR)
				wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return WSAGetLastError();
		}

		wprintf(L"Connected to server.\n");
	}
	return 0;
}

bool Producer::take(int* x, int* y, int* freq) {
	if (bufferStream.eof()) {
		// buffer data has been consumed -> get new data
		if (!fillBuffer()) {
			return false;
		}
	}
	char comma = '\0', comma2 = '\0';
	int throwaway = 0;
	if (x == nullptr) { x = &throwaway; }
	if (y == nullptr) { y = &throwaway; }
	if (freq == nullptr) { freq = &throwaway; }
	return (bufferStream >> *x >> comma >> *y >> comma2 >> *freq) && comma == ',' && comma2 == ',';
}

bool Producer::fillBuffer() {
	if (isFile) {
		// init buffer stream to empty
		bufferStream.str();
		bufferStream.clear();
		// don't read whole file at once (to keep things moving)
		int initialFilePos = file.tellg();
		// parse x,y,freq from file
		// note: parsing into variables is done to have at least some sort of file structure check
		int x = 0, y = 0, freq = 0;
		char comma = '\0', comma2 = '\0';
		while ((file >> x >> comma >> y >> comma2 >> freq) && comma == ',' && comma2 == ',' && ((int)file.tellg() - initialFilePos) < BUFFER_LEN) {
			wprintf(L"Parsed x=%d, y=%d, freq=%d\n", x, y, freq);
			bufferStream << x << comma << y << comma2 << freq << '\n';
		}

		if (file.fail() || bufferStream.fail()) {
			wprintf(L"Error while parsing input file!\n");
			return false;
		}
	}
	else {
		// init buffer stream with incomplete triples from last recv
		bufferStream.str(rest);
		bufferStream.clear();
		
		
		// read some data from the socket or wait for it
		// don't fill whole buffer to allow 0 termination
		char buffer[BUFFER_LEN];
		int bytesRead = 0;
		bytesRead = recv(connectSocket, buffer, BUFFER_LEN - 1, 0);	
		if (bytesRead > 0) {
			printf("Bytes received: %d\n", bytesRead);

			// 0 terminate buffer to allow stringstream handling
			buffer[bytesRead] = '\0';	
			// handle incomplete triples at the end of received data
			for (int i = bytesRead - 1; i > 0; i--) {
				if (buffer[i] == '\n') {
					rest = (char*)malloc(bytesRead - i);
					strcpy_s(rest, bytesRead - i, &buffer[i + 1]);
					buffer[i + 1] = '\0';
					break;
				}
			} // buffer data ends with a complete triple which is 0 terminated, the incomplete triple lies in rest

			bufferStream << buffer;
		}
		else if (bytesRead == 0) {
			printf("Connection closed\n");
			return false;
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			return false;
		}
	}

	return true;
}

Producer::~Producer() {
	free(rest);
}