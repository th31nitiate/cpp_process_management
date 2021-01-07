// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
#pragma data_seg(".shared")
DWORD g_ThreadId = 0;
HHOOK g_hHook = nullptr;
#pragma data_seg()
#pragma comment(linker, "/section:.shared,RWS")


BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, PVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            printf("DLL seems to be loading\n");
            break;
        case DLL_PROCESS_DETACH:
            PostThreadMessage(g_ThreadId, WM_QUIT, 0, 0);
            printf("Dll seems to be unloading\n");
            break;
    }
    return TRUE;
}


extern "C" LRESULT CALLBACK HookFuncation(int code, WPARAM wParam, LPARAM lParam) {
    printf("Running Hook funcation\n");
    if (code == HC_ACTION) {
        auto msg = (MSG*)lParam;
        if (msg->message == WM_CHAR) {
            PostThreadMessage(g_ThreadId, WM_APP, msg->wParam, msg->lParam);
        }
    }
    return CallNextHookEx(g_hHook, code, wParam, lParam);
}



extern "C" void WINAPI SetNotificationThread(DWORD threadId, HHOOK hHook) {
    g_ThreadId = threadId;
    g_hHook = hHook;
    printf("Running setnotification %i and passed in value: %i!\n", g_ThreadId, threadId);
}