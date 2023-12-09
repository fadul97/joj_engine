#include "input_win32.h"

#if PLATFORM_WINDOWS

// Static members
b8 JojPlatform::Win32Input::keys[256] = { 0 };		// Keyboard/Mouse state
b8 JojPlatform::Win32Input::ctrl[256] = { 0 };		// Key release control

i32	JojPlatform::Win32Input::xmouse = 0;			// X mouse position
i32	JojPlatform::Win32Input::ymouse = 0;			// Y mouse position
i16 JojPlatform::Win32Input::mouse_wheel = 0;		// Mouse wheel value

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

short JojPlatform::Win32Input::get_mouse_wheel()
{
	i16 val = mouse_wheel;
	mouse_wheel = 0;
	return val;
}

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

	// Mouse movement
	case WM_MOUSEMOVE:
		xmouse = GET_X_LPARAM(lParam);
		ymouse = GET_Y_LPARAM(lParam);
		return 0;

	// Mouse wheel movement
	case WM_MOUSEWHEEL:
		mouse_wheel = GET_WHEEL_DELTA_WPARAM(wParam);
		return 0;

	// Left mouse button pressed
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		keys[VK_LBUTTON] = true;
		return 0;

	// Middle mouse button pressed
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		keys[VK_MBUTTON] = true;
		return 0;
	
	// Right mouse button pressed
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		keys[VK_RBUTTON] = true;
		return 0;

	// Left mouse button released
	case WM_LBUTTONUP:
		keys[VK_LBUTTON] = false;
		return 0;

	// Middle mouse button released
	case WM_MBUTTONUP:
		keys[VK_MBUTTON] = false;
		return 0;

	// Right mouse button released
	case WM_RBUTTONUP:
		keys[VK_RBUTTON] = false;
		return 0;
	}

	return CallWindowProc(JojPlatform::Win32Window::WinProc, hWnd, msg, wParam, lParam);
}

#endif	// PLATFORM_WINDOWS