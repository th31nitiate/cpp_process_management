// WindowsAPIDemos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <Psapi.h>


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


   // WCHAR buffer[128];
    //::StringCchPrintf(buffer, _countof(buffer), L"This is my string in process %u :: %u :: %u", ::GetCurrentProcessId(), sizeof(buffer), _countof(buffer));
    //::MessageBox(nullptr, buffer, L"String Demo", MB_OK | MB_ICONINFORMATION);

    WCHAR path[MAX_PATH];
    ::GetSystemDirectory(path, _countof(path));
    printf("System directory: %ws\n", path);


    WCHAR name[64];
    DWORD size = _countof(name);
    if (::GetComputerName(name, &size))
        printf("Computer name: %ws (%u)\n", name);



    size = _countof(name);
    if (::GetUserName(name, &size))
        printf("User Name: %ws\n", name);

    CONSOLE_CURSOR_INFO ci;
    if (::GetConsoleCursorInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &ci)) {
        ci.bVisible = FALSE;
        ::SetConsoleCursorInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &ci);
    }

    PERFORMANCE_INFORMATION pi;
    printf("\n");

    for (;;) {
        if (!::GetPerformanceInfo(&pi, sizeof(pi)))
            break;

        printf("Processes: %4u\n", pi.ProcessCount);
        printf("Threads: %5u\n", pi.ThreadCount);
        printf("Handles: %7u\n", pi.HandleCount);
        printf("Committed: %7zu KB\n", pi.CommitTotal >> 10);
        printf("Kernel Pool: %5zu KB\n", pi.KernelTotal >> 10);
        ::Sleep(500);
        printf("\r");
        fflush(stdout);
        //printf("\r\033[5A");
    }

    //SHELLEXECUTEINFO sei = { sizeof(sei) };
    //memset(%sei, 0, sizeof(sei));
    //sei.cbSize = sizeof(sei);
    
    //sei.lpFile = L"c:\\windows\\win.ini";
    //sei.lpVerb = L"open";
    //sei.nShow = SW_SHOWNORMAL;

    //::ShellExecuteEx(&sei);


    ///HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, 2552);
    //if (hProcess) {
    //    ::TerminateProcess(hProcess, 0);
    //}
    //else
    //    return Error("Failed to open process");
    
    return 0;
}

