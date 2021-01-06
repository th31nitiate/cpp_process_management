// BasicSharing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <conio.h>



//If we run two insatnce both or all process will share memeory due to the hMemMap


// Forward funcation declaration
void Read(void* p);
void Write(void* p);

int Error(const char* msg) {
    printf("%s (%u)\n", msg, ::GetLastError());
    return 1;
}

int main()
{

    //This can be viewed and verified from process exploere handles windows
    HANDLE hMemMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1 << 16, L"MySharedMemory");
    if (!hMemMap)
        return Error("Failed to create shared memory");

    void* p = ::MapViewOfFile(hMemMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (!p)
        return Error("Failed in MapViewOfFile");

    printf("Address: 0x%p\n", p);


    //Funcation to allow user control of the programe
    bool quit = false;
    while (!quit) {
        printf("1=write, 2=read, 0=quit: ");
        int selection = _getch();
        printf("%c\n", selection);
        switch (selection) {
        case '1':
            Write(p);
            break;

        case '2':
            Read(p);
            break;

        case '0':
            quit = true;
            break;

        }
    }


    ::UnmapViewOfFile(p);
    ::CloseHandle(hMemMap);
    return 0;

}

//Fucnation for reading and writing to memory
void Read(void* p) {
    printf("%s\n", (const char*)p);
}

void Write(void* p) {
    printf(">> ");
    char text[128];
    gets_s(text);
    strcpy_s((char*)p, _countof(text), text);
}