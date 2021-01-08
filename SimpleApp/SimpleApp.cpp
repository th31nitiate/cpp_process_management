
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	MessageBox(nullptr, L"Currently in a hollowed process", L"ProcHollowed", MB_OK);
	return 0;
}