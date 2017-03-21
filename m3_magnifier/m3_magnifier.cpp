// m3_magnifier.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

bool establishConnection(SOCKET *connectSocket);
void closeConnection(SOCKET s);
bool startMagnifier(HANDLE *magnifierHandle);

int main(int argc, char *argv[])
{
	SOCKET connectSocket = NULL;
	if (!establishConnection(&connectSocket)) {
		return 1;
	}

	// connected to server (processing module)

	HANDLE magnifierHandle = NULL;
	if (!startMagnifier(&magnifierHandle)) {
		return 1;
	}

	// magnifier running

	while (WaitForSingleObject(magnifierHandle, 0) == WAIT_TIMEOUT) {	
		// TODO read x,y data from socket
		// SetCursorPosition(x,y);
	}

	// magnifier has terminated

	CloseHandle(magnifierHandle);
	closeConnection(connectSocket);
	return 0;
}

bool establishConnection(SOCKET *connectSocket) {
	// taken from https://msdn.microsoft.com/en-us/library/windows/desktop/ms737625(v=vs.85).aspx
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		return false;
	}
	//----------------------
	// Create a SOCKET for connecting to server
	*connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientService.sin_port = htons(27016);

	//----------------------
	// Connect to server.
	iResult = connect(*connectSocket, (SOCKADDR *)& clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		iResult = closesocket(*connectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	wprintf(L"Connected to server.\n");
	return true;
}

void closeConnection(SOCKET s) {
	// close connection
	int iResult = closesocket(s);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
	}
	WSACleanup();
}

bool startMagnifier(HANDLE *magnifierHandle) {
	// start the magnifier process
	// note: using 'start magnify.exe' does not work! (Results in wrong PID/handle obtained by getProcessHandle, however no problems with other executables...?)
	// omitting 'start' causes utilman.exe to be launched, which then launches Magnifier.exe, this is why when attempting to obtain the process handle, a timeout with retries is necessary
	system("magnify.exe");

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = NULL;

	wprintf(L"Attempting to get magnifier handle...\n");
	*magnifierHandle = NULL;
	int attempt = -1;
	do {
		attempt++;
		Sleep(500);

		// http://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c
		snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (Process32First(snapshot, &entry) == TRUE) {
			while (Process32Next(snapshot, &entry) == TRUE) {
				if (wcscmp(entry.szExeFile, L"Magnify.exe") == 0) {
					*magnifierHandle = OpenProcess(SYNCHRONIZE, FALSE, entry.th32ProcessID);
					break;
				}
			}
		}
		CloseHandle(snapshot);

	} while (*magnifierHandle == NULL && attempt < 5);

	if (*magnifierHandle == NULL) {
		wprintf(L"Failed to start magnifier!\n");
		return false;
	}

	wprintf(L"Magnifier handle obtained!\n");
	return true;
}