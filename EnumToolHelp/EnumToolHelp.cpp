#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>


int Error(const char *msg) {
	printf("%s (%u)", msg, GetLastError());
	return 1;
}

int main() {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return Error("Failed to create snapshot");

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);

	if (!Process32First(hSnapshot, &pe))
		return Error("Failed in Process32First");

	do {
		printf("PID: %6u (PPID: %6u) Threads: %3u %ws\n", pe.th32ProcessID, pe.th32ParentProcessID, pe.cntThreads, pe.szExeFile);
	} while (Process32Next(hSnapshot, &pe));


	CloseHandle(hSnapshot);
	return 0;

}