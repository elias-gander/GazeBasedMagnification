// m3_magnification.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

VOID CALLBACK WaitOrTimerCallback(
	_In_ PVOID   lpParameter,
	_In_ BOOLEAN TimerOrWaitFired
);
bool startMagnifier(HANDLE *magnifierHandle);

bool magnifierRunning = false;
int main(int argc, char *argv[])
{
	HANDLE magnifierHandle = NULL;
	while (!startMagnifier(&magnifierHandle)) {
		wprintf(L"Failed to start magnifier...Press Enter to retry");
		std::getchar();
	}
	magnifierRunning = true;
	// enable callback setting magnifierRunning to false if magnifier terminates
	HANDLE hNewHandle;
	RegisterWaitForSingleObject(&hNewHandle, magnifierHandle, WaitOrTimerCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);


	Producer processModule(argv[1], atoi(argv[2]));
	processModule.hookup();

	bool okay = true;
	int x, y;
	while (okay && magnifierRunning) {
		okay = processModule.take(&x, &y, NULL);
		SetCursorPos(x, y);
	}

	wprintf(L"done...press enter to terminate");
	std::getchar();

	return 0;
}

// http://stackoverflow.com/questions/3556048/how-to-detect-win32-process-creation-termination-in-c
VOID CALLBACK WaitOrTimerCallback(
	_In_ PVOID   lpParameter,
	_In_ BOOLEAN TimerOrWaitFired
) {
	magnifierRunning = false;
}

bool startMagnifier(HANDLE *magnifierHandle) {
	// start the magnifier process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	wchar_t cmd[] = L"magnify.exe";

	if (!CreateProcess(NULL,   // No module name (use command line)
		cmd,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		wprintf(L"CreateProcess failed (%d).\n", GetLastError());
	}

	// for some reason the handle set by CreateProcess is not the handle of the actual magnifier...(works fine for other programs)
	// try to obtain real magnifier handle by finding active process
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


