#include "input_win32.h"

#if PLATFORM_WINDOWS

// Static members
b8 JojPlatform::Win32Input::keys[256] = { 0 };		// Keyboard/Mouse state
b8 JojPlatform::Win32Input::ctrl[256] = { 0 };		// Key release control

JojPlatform::Win32Input::Win32Input()
{
	/* ATTENTION: assumes that the window has already been created with a call to window->create();
  	 * change the window procedure of the active window to InputProc
	 */
	SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)JojPlatform::Win32Input::InputProc);
}

JojPlatform::Win32Input::~Win32Input()
{
	// returns to use the Window Procedure of the Window class
	SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)JojPlatform::Win32Window::WinProc);
}

// -------------------------------------------------------------------------------

b8 JojPlatform::Win32Input::is_key_press(u32 vkcode)
{
	if (ctrl[vkcode])
	{
		if (is_key_down(vkcode))
		{
			ctrl[vkcode] = false;
			return true;
		}
	}
	else if (is_key_up(vkcode))
	{
		ctrl[vkcode] = true;
	}

	return false;
}

// -------------------------------------------------------------------------------

LRESULT CALLBACK JojPlatform::Win32Input::InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// Key pressed
	case WM_KEYDOWN:
		keys[wParam] = true;
		return 0;

		// Key released
	case WM_KEYUP:
		keys[wParam] = false;
		return 0;
	}

	return CallWindowProc(JojPlatform::Win32Window::WinProc, hWnd, msg, wParam, lParam);
}

#endif	// PLATFORM_WINDOWS