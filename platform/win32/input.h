#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "window.h"

namespace JojPlatform
{
	class Input
	{
	public:
		Input();
		~Input();

		b8 is_key_down(u32 vkcode);	// Checks if key/button is pressed
		b8 is_key_up(u32 vkcode);	// Checks if key/button is released
		b8 is_key_pressed(u32 vkcode);	// Register press only after release

		i32 get_xmouse();			// Returns X-axis mouse position
		i32 get_ymouse();			// Returns Y-axis mouse position
		i16 get_mouse_wheel();			// Returns mouse wheel rotation

		// Handle Window events
		static LRESULT CALLBACK InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	private:
		static b8 keys[256];			// Keyboard/Mouse key states
		static b8 ctrl[256];			// Keyrelease control

		static i32 xmouse;				// X mouse position
		static i32 ymouse;				// Y mouse position
		static i16 mouse_wheel;			// Mouse wheel value
	};

	// Returns true if key is pressed
	inline b8 Input::is_key_down(u32 vkcode)
	{ return keys[vkcode]; }

	//  Returns true if key is released
	inline b8 Input::is_key_up(u32 vkcode)
	{ return !(keys[vkcode]); }

	// Returns X-axis mouse position
	inline i32 Input::get_xmouse()
	{ return xmouse; }

	// Returns Y-axis mouse position
	inline i32 Input::get_ymouse()
	{ return ymouse; }

}   // namespace JojPlatform

#endif // PLATFORM_WINDOWS
