// WinVersion.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//Initial return vlaue before appending manfiest was 6.2.9200, then it went to 6.3.9300 as the intended verison number

#define BUILD_WINDOWS
#include <iostream>
#include <Windows.h>
#include <stdio.h>



int main()
{
	OSVERSIONINFO vi = { sizeof(vi) };
	::GetVersionEx(&vi);

	printf("%u.%u.%u\n", vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber);

	return 0;
}

