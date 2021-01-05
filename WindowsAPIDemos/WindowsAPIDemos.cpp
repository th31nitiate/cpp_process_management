// WindowsAPIDemos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>


int Error(const char* msg) {
    printf("%s (%u)\n", msg, ::GetLastError());
    return 1;
}


int main()
{
    SYSTEM_INFO si;
    ::GetNativeSystemInfo(&si);

    printf("Processors: %u\n", si.dwNumberOfProcessors);
    printf("Page Size; %u bytes\n", si.dwPageSize);
    printf("Processor mask: 0x%zX\n", si.dwActiveProcessorMask);
    printf("Minimum process address: 0x%p\n", si.lpMinimumApplicationAddress);
    printf("Maximum process address: 0x%p\n", si.lpMaximumApplicationAddress);


    WCHAR buffer[128];
    ::StringCchPrintf(buffer, _countof(buffer), L"This is my string in process %u :: %u :: %u", ::GetCurrentProcessId(), sizeof(buffer), _countof(buffer));
    ::MessageBox(nullptr, buffer, L"String Demo", MB_OK | MB_ICONINFORMATION);

    WCHAR path[MAX_PATH];
    ::GetSystemDirectory(path, _countof(path));
    printf("System directory: %ws\n", path);


    WCHAR computerName[MAX_COMPUTERNAME_LENGTH];
    DWORD len = _countof(computerName);
    if (::GetComputerName(computerName, &len))
        printf("Computer name: %ws (%u)\n", computerName, len);

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    //memset(%sei, 0, sizeof(sei));
    //sei.cbSize = sizeof(sei);
    
    sei.lpFile = L"c:\\windows\\win.ini";
    sei.lpVerb = L"open";
    sei.nShow = SW_SHOWNORMAL;

    ::ShellExecuteEx(&sei);


    ///HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, 2552);
    //if (hProcess) {
    //    ::TerminateProcess(hProcess, 0);
    //}
    //else
    //    return Error("Failed to open process");
    
    return 0;
}

