//It is  important to note that we require the correct architecture to perform this technique
//This is currently geared towards 32-bit systems
//RemoteThread.exe pid C:\Users\%username%\Source\Repos\WindowsAPIDemos\x64\Debug\injected.dll

#include <Windows.h>
#include <stdio.h>


int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	return 1;
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

	if(!WriteProcessMemory(hProcess, buffer, argv[2], strlen(argv[2]), nullptr))
		return Error("Failed to write memory");

	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryA"), buffer, 0, nullptr);
	if (!hThread)
		return Error("Failed to create thread");


	return 0;


}
