// Handles.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>



int main()
{
    HANDLE hEvent = ::CreateEvent(nullptr, TRUE, FALSE, L"InitialEvent");
    if (hEvent == nullptr)
        printf("Failed to create event (%u)\n", ::GetLastError());
    else {
        ::Sleep(INFINITE);
        ::SetEvent(hEvent);
        ::CloseHandle(hEvent);
    }

   //:SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);



    return 0;
}