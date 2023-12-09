#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "window_win32.h"

namespace JojPlatform
{
	class Win32Input
	{
	public:
		Win32Input();
		~Win32Input();

		b8 is_key_down(u32 vkcode);		// Checks if key is pressed
		b8 is_key_up(u32 vkcode);		// Checks if key is released
		b8 is_key_press(u32 vkcode);	// Register press only after release		

		static LRESULT CALLBACK InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	private:
		static b8 keys[256];			// Keyboard/Mouse key states
		static b8 ctrl[256];			// Keyrelease control
	};

	// Returns true if key is pressed
	inline b8 Win32Input::is_key_down(u32 vkcode)
	{ return keys[vkcode]; }

	//  Returns true if key is released
	inline b8 Win32Input::is_key_up(u32 vkcode)
	{ return !(keys[vkcode]); }

}   // JojPlatform

#endif	// PLATFORM_WINDOWS