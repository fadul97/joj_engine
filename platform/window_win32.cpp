#include "window_win32.h"

#include <iostream>

#define DEBUG

#ifdef DEBUG
#include <sstream>
#endif // DEBUG

void print_window_win32(u32 n)
{
#ifdef DEBUG
	std::wstringstream text;
	text << L"Hello from print_window_win32(" << n << ")!\n";
	OutputDebugStringW(text.str().c_str());
#else
	std::cout << "Hello from print_window_win32(" << n << ")!\n";
#endif // DEBUG

}