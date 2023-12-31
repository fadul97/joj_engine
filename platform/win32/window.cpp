#include "window.h"

#if PLATFORM_WINDOWS

#include <string>

// Static members
void (*JojPlatform::Window::on_focus)() = nullptr;		// Do nothing when gaining focus
void (*JojPlatform::Window::lost_focus)() = nullptr;	// Do nothing when losing focus

JojPlatform::Window::Window()
{
	id = 0;									// Null ID because the window does not exist yet
	hdc = { 0 };							// Device context
	width = GetSystemMetrics(SM_CXSCREEN);  // Window occupies the entire screen (fullscreen)
	height = GetSystemMetrics(SM_CYSCREEN); // Window occupies the entire screen (fullscreen)
	title = std::string("Joj Window");		// Default window title
	rect = { 0, 0, 0, 0 };                  // Window client area
	icon = LoadIcon(NULL, IDI_APPLICATION); // Default icon for an application
	cursor = LoadCursor(NULL, IDC_ARROW);   // Default cursor for an application
	color = RGB(0, 0, 0);					// Default background color is black
	style = WS_POPUP | WS_VISIBLE;          // Style for fullscreen
	mode = WindowMode::FULLSCREEN;			// Default mode is fullscreen
	xpos = 0;                               // Initial window position on the x-axis
	ypos = 0;                               // Initial window position on the y-axis
	xcenter = width / 2;                    // Window center on the x-axis
	ycenter = height / 2;                   // Window center on the y-axis
}

JojPlatform::Window::~Window()
{
	// Release rendering context
	//if (rc)
	//	wglDeleteContext(rc);

	// Release device context
	//if (hdc)
	//	ReleaseDC(id, hdc);
}

void JojPlatform::Window::set_mode(WindowMode mode)
{
	this->mode = mode;

	// Window mode
	if (mode == WindowMode::WINDOWED)
	{
		style = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE;
	}
	// Bordeless window
	else
	{
		style = WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE;
	}
}

void JojPlatform::Window::set_size(i32 width, i32 height)
{
	// Window size
	this->width = width;
	this->height = height;

	// Adjusts window position to the center of the screen
	xpos = (GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2);
	ypos = (GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2);
	
	// Calculates window center position
	xcenter = xpos + (width / 2);
	ycenter = ypos + (height / 2);
}

/*
 *	@brief Displays the text at the (x,y) position on the screen using the specified color,
 *	it uses Windows GDI (slow) and should only be used for debugging.
 */
void JojPlatform::Window::print_on_window(std::string text, i16 x, i16 y, COLORREF color)
{
	// Set text color
	SetTextColor(hdc, color);

	// Set background text to be transparent
	SetBkMode(hdc, TRANSPARENT);

	// Show text
	TextOut(hdc, x, y, text.c_str(), (int)text.size());
}

b8 JojPlatform::Window::create()
{
	// Window class name
	const char* joj_window_class = "JOJ_WINDOW_CLASS";

	// Application ID
	HINSTANCE app_id = GetModuleHandle(NULL);

	// Failed to get application ID
	if (!app_id)
	{
		// TODO: Use own logger
		OutputDebugString("Failed to get module handle.\n");
		return false;
	}

	// Define window class
	WNDCLASSEX wndClass = { };

	// Do not register the "JOJ_WINDOW_CLASS" class multiple times
	if (!GetClassInfoExA(app_id, joj_window_class, &wndClass))
	{
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = JojPlatform::Window::WinProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = app_id;
		wndClass.hIcon = icon;
		wndClass.hCursor = cursor;
		wndClass.hbrBackground = (HBRUSH)CreateSolidBrush(color);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = joj_window_class;
		wndClass.hIconSm = icon;

		// Register "JOJ_WINDOW_CLASS" class
		if (!RegisterClassEx(&wndClass))
		{
			// TODO: Use own logger
			OutputDebugString("Failed to register window class.\n");
			return false;
		}
	}

	// Create a window basend on the "WindowApp" class
	id = CreateWindowEx(
		NULL,				// Extras styles
		joj_window_class,	// Window class name
		title.c_str(),		// Window title
		style,				// Window style
		xpos, ypos,			// Initial (x, y) position
		width, height,		// Window width and height
		NULL,				// Parent window ID
		NULL,				// Menu ID
		app_id,				// application ID
		NULL);				// Creatin parameters

	if (!id)
	{
		// TODO: Use own logger
		OutputDebugString("Failed to create a window.\n");
		return false;
	}

	/*
	 * When using windowed mode, it is necessary to take into account that the bars
	 * and borders take up space in the window. The code below adjusts the size
	 * of the window so that the client area is the same size
	 * (width x height)
	*/

	if (mode == WindowMode::WINDOWED)
	{
		// Rectangle with the desired size
		RECT new_rect = { 0, 0, width, height };

		// Adjusts rectangle size
		if (!AdjustWindowRectEx(&new_rect,
			GetWindowStyle(id),
			GetMenu(id) != NULL,
			GetWindowExStyle(id)))
		{
			// TODO: Use own logger
			OutputDebugString("Could not adjust window rect ex.\n");
		}

		// Updated window position
		xpos = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((new_rect.right - new_rect.left) / 2);
		ypos = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((new_rect.bottom - new_rect.top) / 2);

		// Resize window with a call to MoveWindow
		if (!MoveWindow(
			id,									// Window ID
			xpos,								// X position
			ypos,								// Y position
			new_rect.right - new_rect.left,		// Width
			new_rect.bottom - new_rect.top,		// Height
			TRUE))								// Repaint
		{
			// TODO: Use own logger
			OutputDebugString("Could not move window.\n");
		}
	}

	// Get device context
	hdc = GetDC(id);

	// Failed to get device context
	if (!hdc)
	{
		// TODO: Use own logger
		OutputDebugString("Failed to get device context.\n");
		return false;
	}

	// Get client area size
	if (!GetClientRect(id, &rect))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to get client area size.\n");
		return false;
	}

	// Returns initialization status (successful or not)
	return (id ? true : false);
}

LRESULT CALLBACK JojPlatform::Window::WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	// Window closed
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

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
	//case WM_DESTROY:
	//	// Send a WM_QUIT message to close application
	//	PostQuitMessage(0);
	//	return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif // PLATFORM_WINDOWS
