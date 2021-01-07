// WindowsHooks.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <Psapi.h>


bool EnableDebugPriviledge() { //Need to gain a better understanding of the token privileges process
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		return false;

	BOOL success = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
	CloseHandle(hToken);

	return success && GetLastError() == ERROR_SUCCESS;
}


DWORD FindMainNotepadThread() {
	auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	DWORD tid = 0;
	THREADENTRY32 th32;
	th32.dwSize = sizeof(th32);

	Thread32First(hSnapshot, &th32);
	do {
		auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, th32.th32OwnerProcessID);
		if (hProcess) {
			WCHAR name[MAX_PATH];
			if (GetProcessImageFileName(hProcess, name, MAX_PATH) > 0) {
				auto bs = wcsrchr(name, L'\\');
				if (bs && _wcsicmp(bs, L"\\notepad.exe") == 0) {
					tid = th32.th32ThreadID;
					break;
				}
			}
			CloseHandle(hProcess);
		}
	} while (Thread32Next(hSnapshot, &th32));
	CloseHandle(hSnapshot);
	return tid;
}

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	return 1;
}


int main() {

	EnableDebugPriviledge(); //Allows you to see more information regarding specific processes with admin privs
	DWORD tid = FindMainNotepadThread();
	if (tid == 0)
		return Error("Failed to locate Notepad\n");

	auto hDLL = LoadLibrary(L"InjectedHook.dll");
	if (!hDLL)
		return Error("Failed to locate Dll\n");


	printf("Notepad thread value: %i!\n", tid);
	auto setNotify = (void (WINAPI*)(DWORD, HHOOK))GetProcAddress(hDLL, "SetNotificationThread");
	if (!setNotify)
		return Error("Failed to install notify\n");
	else
		printf("Loaded Set Notify!!!\n");

	auto hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetProcAddress(hDLL, "HookFuncation"), hDLL, tid);
	if (!hHook)
		return Error("Failed to install hook\n");
	else
		printf("Loaded Hook funcation!!\n");


	setNotify(GetCurrentThreadId(), hHook);
	PostThreadMessage(tid, WM_NULL, 0, 0);


	
	MSG msg;
	//Need to investigate why the GetMessage funcation is not working, the while loop seems to not start
	while (GetMessage(&msg, nullptr, 0, 0)) {
		printf("In while loop in which we get message");
		if (msg.message == WM_APP) {
			printf("%c", (int)msg.wParam);
			if (msg.wParam == 13)
				printf("\n");
		}
	}
	UnhookWindowsHookEx(hHook);
	FreeLibrary(hDLL);

	return 0;
}