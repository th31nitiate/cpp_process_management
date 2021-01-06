// SingleInstance.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>

int main()
{
    HANDLE hMutex = ::CreateMutex(nullptr, FALSE, L"MySingleInstanceMutex");
    if (!hMutex) {
        printf("Error creating mutex!\n");
        return 1;
    }

    if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        printf("Second instance... shutting down\n");
        return 0;
    }



    printf("First instance ...\n");
    char dummy[4];
    gets_s(dummy);

    ::CloseHandle(hMutex);
}
