// APCInject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//Not properly funcational with explorer but pretty good with normal processes as explorer crash thought the
//File in its self is injected. Use NtQueueApcThread for further funcationality such as passing arguments to the dll

#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <vector>

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	return 1;
}


std::vector<DWORD> GetProcessThreads(DWORD pid) {
	std::vector<DWORD> tids;

	auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return tids;

	THREADENTRY32 te = { sizeof(te) };
	if (Thread32First(hSnapshot, &te)) {
		do {
			if (te.th32OwnerProcessID == pid) {
				tids.push_back(te.th32ThreadID);
			}
		} while (Thread32Next(hSnapshot, &te));
	}

	CloseHandle(hSnapshot);
	return tids;
}

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("Usage: remotethread <pid> <dllpath>\n");
		return 0;
	}


	int pid = atoi(argv[1]);

	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD, FALSE, pid);
	if (!hProcess)
		return Error("Error opening process");

	void* buffer = VirtualAllocEx(hProcess, nullptr, 1 << 12, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!buffer)
		return Error("Failed to allocate memory");

	if (!WriteProcessMemory(hProcess, buffer, argv[2], strlen(argv[2]), nullptr))
		return Error("Failed to write memory");

	auto tids = GetProcessThreads(pid);
	if (tids.empty()) {
		printf("Failed to locate threads in process: %u\n", pid);
		return 1;
	}

	for (const DWORD tid : tids) {
		HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, tid);
		if (hThread) {
			QueueUserAPC((PAPCFUNC)GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryA"), hThread, (ULONG_PTR)buffer);
			CloseHandle(hThread);
		}
	}
	printf("APC sent!\n");

	CloseHandle(hProcess);

	return 0;
}
