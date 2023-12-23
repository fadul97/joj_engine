#include "input.h"

#if PLATFORM_WINDOWS

// Static members
b8 JojPlatform::Input::keys[256] = { 0 };		// Keyboard/Mouse state
b8 JojPlatform::Input::ctrl[256] = { 0 };		// Key release control

i32	JojPlatform::Input::xmouse = 0;			// X mouse position
i32	JojPlatform::Input::ymouse = 0;			// Y mouse position
i16 JojPlatform::Input::mouse_wheel = 0;		// Mouse wheel value

JojPlatform::Input::Input()
{
	/* ATTENTION: assumes that the window has already been created with a call to window->create();
  	 * change the window procedure of the active window to InputProc
	 */
	SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)JojPlatform::Input::InputProc);
}

JojPlatform::Input::~Input()
{
	// returns to use the Window Procedure of the Window class
	SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)JojPlatform::Window::WinProc);
}

b8 JojPlatform::Input::is_key_pressed(u32 vkcode)
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

short JojPlatform::Input::get_mouse_wheel()
{
	i16 val = mouse_wheel;
	mouse_wheel = 0;
	return val;
}

LRESULT CALLBACK JojPlatform::Input::InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	return CallWindowProc(JojPlatform::Window::WinProc, hWnd, msg, wParam, lParam);
}

#endif	// PLATFORM_WINDOWS