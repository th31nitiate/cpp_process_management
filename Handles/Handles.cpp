// Handles.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <string>


int main()
{
    HANDLE hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
    printf("HANDLE: 0x%p\n", hEvent);

    PROCESS_INFORMATION pi;
    STARTUPINFO si = { sizeof(si) };
    //WCHAR name[] = L"Notepad"; //Will search system paths to try find notepad if full path not provided
    std::wstring name(L"Notepad "); //Use this to assist in passing the handle between process to enabel sharing
    name += std::to_wstring((long long)hEvent);

    ::SetHandleInformation(hEvent, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    if (::CreateProcess(nullptr, (WCHAR*)name.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        printf("PID: %u\n", pi.dwProcessId);
        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
    }

   //:SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);



    return 0;
}