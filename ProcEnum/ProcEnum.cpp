//Main thing to rtemeber is process exploere is always there to better understand what information to look for
//The struct can then be used to further explore how to use the information in the porgrame.


#include <Windows.h>
#include <stdio.h>
#include <string>
#include <TlHelp32.h>
#include <WtsApi32.h>
#include <Psapi.h>

#pragma comment(lib, "wtsapi32")
#pragma comment(lib, "ntdll")

#define STATUS_BUFFER_TOO_SMALL 0xC0000004

enum SYSTEM_INFORMATION_CLASS {
	SystemExtendedProcessInformation = 57
};

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MAximumLength;
	PWSTR Buffer;
} UNICODE_STRING;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER WorkingSetPrivateSize;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER SpareLi2;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	LARGE_INTEGER OtherTransferCount;
	LARGE_INTEGER SpareLi3;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	int BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG ReadOperationCount;
	ULONG SpareUl1;
	ULONG HandleCount;
	ULONG WriteOperationCount;
	ULONG SpareUl2;
	ULONG SessionId;
	ULONG OtherOperationCount;
	ULONG SpareUl3;
	ULONG_PTR UniqueProcessKey;
	ULONG_PTR PeakVirtualSize;
	ULONG_PTR VirtualSize;
	ULONG PageFaultCount;
	ULONG_PTR PeakWorkingSetSize;
	ULONG_PTR WorkingSetSize;
	ULONG_PTR QuotaPeakPagedPoolUsage;
	ULONG_PTR QuotaPagedPoolUsage;
	ULONG_PTR QuotaPeakNonPagedPoolUsage;
	ULONG_PTR QuotaNonPagedPoolUsage;
	ULONG_PTR PagefileUsage;
	ULONG_PTR PeakPagefileUsage;
	ULONG_PTR PrivatePageCount;
} SYSTEM_PROCESS_INFORMATION;

extern "C" NTSTATUS NTAPI NtQuerySystemInformation(
	_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
	_In_ ULONG SystemInformationLength,
	_Out_opt_ PULONG ReturnLength
);




//int ProcEnumWithToolhelp();
//int ProcEnumWithWTS();
int ProcEnumWithEnumProc();
int ProcEnumWithNtQuerySystem();

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

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	return 1;
}

int Usage() {
	printf("Usage: procenum <method>\n");
	printf("\t<method>: 1 - use ToolHelp\n");
	printf("\t<method>: 2 - use WTS\n");
	printf("\t<method>: 3 - use EnumProcess\n");
	printf("\n<method>: 4 - use NtQuerySystemInformation\n");

	return 0;
}

int main(int argc, const char* argv[]) {
	if (argc == 1)
		return Usage();


	EnableDebugPriviledge();

	switch (atoi(argv[1])) {
		case 1:
			printf("Using ToolHelp... \n");
			//return ProcEnumWithToolhelp();

		case 2:
			printf("Using WTS... \n");
			//return ProcEnumWithWTS();

		case 3:
			printf("Using EnumProcesses... \n");
			return ProcEnumWithEnumProc();

		case 4:
			printf("Using NtQuerySystemInformation... \n");
			return ProcEnumWithNtQuerySystem();
	}

	printf("Unknown option.... %s\n", argv[1]);
	return Usage();

}

int ProcEnumWithEnumProc() {
	DWORD size = 512 * sizeof(DWORD);
	DWORD* ids = nullptr;
	DWORD needed = 0;


	for (;;) {
		ids = (DWORD*)realloc(ids, size);
		if (ids == nullptr)
			break;

		if (!EnumProcesses(ids, size, &needed)) {
			free(ids);
			return Error("Failed in EnumProcesses");
		}
		if (size > needed)
			break;

		size = needed + sizeof(DWORD) * 16;
	}
	if (ids == nullptr)
		return Error("Out of memory");

	WCHAR name[MAX_PATH];
	for (DWORD i = 0; i < needed / sizeof(DWORD); i++) {
		DWORD id = ids[i];
		printf("PID: %6u", id);
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, id);
		if (hProcess) {
			DWORD size = _countof(name);
			if (QueryFullProcessImageName(hProcess, 0, name, &size)) {
				printf(" %ws ", name);
			}
			CloseHandle(hProcess);
		}
		printf("\n");
	}
	free(ids);
	return 0;

}

int ProcEnumWithNtQuerySystem() {
	// allocate large-enough buffer
	ULONG size = 1 << 18;
	void* buffer = nullptr;

	for (;;) {
		buffer = realloc(buffer, size);
		if (!buffer)
			return 1;

		ULONG needed;
		NTSTATUS status = NtQuerySystemInformation(SystemExtendedProcessInformation, buffer, size, &needed);
		if (status == 0)
			break;

		if (status == STATUS_BUFFER_TOO_SMALL) {
			size = needed + (1 << 12);
			continue;
		}

		return status;
	}

	auto p = (SYSTEM_PROCESS_INFORMATION*)buffer;
	for (;;) {
		printf("PID: %6u PPID: %6u, Session: %u, Threads: %3u %ws\n", 
			HandleToULong(p->UniqueProcessId), HandleToULong(p->InheritedFromUniqueProcessId),
			p->SessionId, p->NumberOfThreads, 
			p->ImageName.Buffer ? p->ImageName.Buffer : L""); //At some point invetigate error caused when enum specific processes as shown with InheritedFromUniqueProcess	
		//Printf error most likely caused by issues in system process struct

		if (p->NextEntryOffset == 0)
			break;

		p = (SYSTEM_PROCESS_INFORMATION*)((BYTE*)p + p->NextEntryOffset);
	}
	free(buffer);

	return 0;
}