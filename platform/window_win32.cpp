#include "window_win32.h"

#if PLATFORM_WINDOWS

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

// Static members
void (*JojPlatform::Win32Window::on_focus)() = nullptr;		// Do nothing when gaining focus
void (*JojPlatform::Win32Window::lost_focus)() = nullptr;	// Do nothing when losing focus

JojPlatform::Win32Window::Win32Window()
{
	id = 0;									// Null ID because the window does not exist yet
	width = GetSystemMetrics(SM_CXSCREEN);  // Window occupies the entire screen (fullscreen)
	height = GetSystemMetrics(SM_CYSCREEN); // Window occupies the entire screen (fullscreen)
	icon = LoadIcon(NULL, IDI_APPLICATION); // Default icon for an application
	cursor = LoadCursor(NULL, IDC_ARROW);   // Default cursor for an application
	color = RGB(0, 0, 0);					// Default background color is black
	title = std::string("Windows App");		// Default window title
	style = WS_POPUP | WS_VISIBLE;          // Style for fullscreen
	mode = JojPlatform::FULLSCREEN;         // Default mode is fullscreen
	xpos = 0;                               // Initial window position on the x-axis
	ypos = 0;                               // Initial window position on the y-axis
	xcenter = width / 2;                    // Window center on the x-axis
	ycenter = height / 2;                   // Window center on the y-axis
	hdc = { 0 };                            // Device context
	rect = { 0, 0, 0, 0 };                  // Window client area
}

JojPlatform::Win32Window::~Win32Window()
{
	// Release device context
	if (hdc)
		ReleaseDC(id, hdc);
}

void JojPlatform::Win32Window::set_mode(u32 mode)
{
	this->mode = mode;

	// Window mode
	if (mode == WINDOWED)
	{
		style = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE;
	}
	// Bordeless window
	else
	{
		style = WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE;
	}
}

void JojPlatform::Win32Window::set_size(i16 width, i16 height)
{
	// Window size
	this->width = width;
	this->height = height;

	// Calculates window center position
	xcenter = this->width / 2;
	ycenter = this->height / 2;

	// Adjusts window position to the center of the screen
	xpos = (GetSystemMetrics(SM_CXSCREEN) / 2) - (this->width / 2);
	ypos = (GetSystemMetrics(SM_CYSCREEN) / 2) - (this->height / 2);
}

/*
 *	@brief Displays the text at the (x,y) position on the screen using the specified color,
 *	it uses Windows GDI (slow) and should only be used for debugging.
 */
void JojPlatform::Win32Window::print_on_window(std::string text, i16 x, i16 y, COLORREF color)
{
	// Set text color
	SetTextColor(hdc, color);

	// Set background text to be transparent
	SetBkMode(hdc, TRANSPARENT);

	// Show text
	TextOut(hdc, x, y, text.c_str(), (int)text.size());
}

b8 JojPlatform::Win32Window::create()
{
	// Application ID
	HINSTANCE appId = GetModuleHandle(NULL);

	// Define window class
	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = JojPlatform::Win32Window::WinProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = appId;
	wndClass.hIcon = icon;
	wndClass.hCursor = cursor;
	wndClass.hbrBackground = (HBRUSH)CreateSolidBrush(color);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "WindowApp";
	wndClass.hIconSm = icon;

	// Register "WindowApp" class
	if (!RegisterClassEx(&wndClass))
		return false;

	// Create a window basend on the "WindowApp" class
	id = CreateWindowEx(
		NULL,					// Extras styles
		"WindowApp",			// Window class name
		title.c_str(),			// Window title
		style,					// Window style
		xpos, ypos,				// Initial (x, y) position
		width, height,			// Window width and height
		NULL,					// Parent window ID
		NULL,					// Menu ID
		appId,					// application ID
		NULL);					// Creatin parameters

	/*
	 * When using windowed mode, it is necessary to take into account that the bars
	 * and borders take up space in the window. The code below adjusts the size
	 * of the window so that the client area is the same size
	 * (width x height)
	*/

	if (mode == WINDOWED)
	{
		// Rectangle with the desired size
		RECT new_rect = { 0, 0, width, height };

		// Adjusts rectangle size
		AdjustWindowRectEx(&rect,
			GetWindowStyle(id),
			GetMenu(id) != NULL,
			GetWindowExStyle(id));

		// Updated window position
		xpos = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((new_rect.right - new_rect.left) / 2);
		ypos = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((new_rect.bottom - new_rect.top) / 2);

		// Resize window with a call to MoveWindow
		MoveWindow(
			id,									// Window ID
			xpos,								// X position
			ypos,								// Y position
			new_rect.right - new_rect.left,		// Width
			new_rect.bottom - new_rect.top,		// Height
			TRUE);								// Repaint
	}

	// Get device context
	hdc = GetDC(id);

	// Get client area size
	GetClientRect(id, &rect);

	// Returns initialization status (successful or not)
	return (id ? true : false);
}

LRESULT CALLBACK JojPlatform::Win32Window::WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// Window lost focus
	case WM_KILLFOCUS:
		if (lost_focus)
			lost_focus();
		return 0;

		// Window regained focus
	case WM_SETFOCUS:
		if (on_focus)
			on_focus();
		return 0;

		// Window was destroyed
	case WM_DESTROY:
		// Send a WM_QUIT message to close application
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif	// PLATFORM_WINDOWS
