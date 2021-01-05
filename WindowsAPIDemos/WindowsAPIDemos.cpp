// WindowsAPIDemos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <stdio.h>


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


    HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, 2552);
    if (hProcess) {
        ::TerminateProcess(hProcess, 0);
    }
    else
        return Error("Failed to open process");

        return 0;
}

