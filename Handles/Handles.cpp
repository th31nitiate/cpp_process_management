// Handles.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <string>


int main()
{
    HANDLE hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
    printf("HANDLE: 0x%p\n", hEvent);

    HANDLE hProcess = ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, 7320);
    if (!hProcess) {
        printf("Error opening process (%u)\n", ::GetLastError());
        return 1;
    }


    HANDLE hTarget;
    if (::DuplicateHandle(::GetCurrentProcess(), hEvent, hProcess, &hTarget, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        printf("Success!\n");
    }

   //:SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);



    return 0;
}