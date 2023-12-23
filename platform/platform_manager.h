#pragma once

#include "defines.h"

#include <memory>

#if PLATFORM_WINDOWS
#include "win32/window.h"
#include "win32/input.h"
#include "win32/timer.h"
#endif // PLATFORM_WINDOWS

namespace JojPlatform
{
	class PlatformManager
	{
	public:
		PlatformManager();
		~PlatformManager();

		b8 init(i32 width = 1366, i32 height = 768, std::string title = "Joj Engine");	// Initialize platform specifics
		void process_events(MSG msg);													// Process Window events

		b8 is_key_down(u32 key_code);		// Check if key is pressed
		b8 is_key_up(u32 key_code);			// Check if key is released
		b8 is_key_pressed(u32 key_code);	// Register press only after release

		i32 get_xmouse();				// Return X-axis mouse position
		i32 get_ymouse();				// Return Y-axis mouse position
		i16 get_mouse_wheel();			// Return mouse wheel rotation

		void begin_period();	// Adjust sleep resolution to 1 millisecond
		void end_period();		// Return sleep resolution to original value
		void start_timer();		// Start/resume counting time
		f32 reset_timer();		// Restarts timer counter and returns elapsed time

		void set_on_focus(void(*func)());	// Set function to be executed when wubdiw regains focus
		void set_lost_focus(void(*func)());	// Set function to be executed when wubdiw loses focus

#if PLATFORM_WINDOWS
		// Change window procedure to new_win_proc
		void change_window_procedure(HWND window_id, u32 index, LONG_PTR new_win_proc);
#endif // PLATFORM_WINDOWS

	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Input> input;
		std::unique_ptr<Timer> timer;
	};

	// Check if key is pressed
	inline b8 PlatformManager::is_key_down(u32 key_code)
	{ return input->is_key_down(key_code); }
	
	// Check if key is released
	inline b8 PlatformManager::is_key_up(u32 key_code)
	{ return input->is_key_up(key_code); }

	// Register press only after release
	inline b8 PlatformManager::is_key_pressed(u32 key_code)
		{ return input->is_key_pressed(key_code); }

	// Return X-axis mouse position
	inline i32 PlatformManager::get_xmouse()
	{ return input->get_xmouse(); }

	// Return Y-axis mouse position
	inline i32 PlatformManager::get_ymouse()
	{ return input->get_ymouse(); }

	// Return mouse wheel rotation
	inline i16 PlatformManager::get_mouse_wheel()
	{ return input->get_mouse_wheel(); }

	// Adjust sleep resolution to 1 millisecond
	inline void PlatformManager::begin_period()
	{ timer->time_begin_period(); }

	// Return sleep resolution to original value
	inline void PlatformManager::end_period()
	{ timer->time_end_period(); }

	// Start/resume counting time
	inline void PlatformManager::start_timer()
	{ timer->start(); }

	// Restarts timer counter and returns elapsed time
	inline f32 PlatformManager::reset_timer()
	{ timer.reset(); }

	// Set function to be executed when wubdiw regains focus
	inline void PlatformManager::set_on_focus(void(*func)())
	{ window->set_on_focus(func); }

	// Set function to be executed when wubdiw loses focus
	inline void PlatformManager::set_lost_focus(void(*func)())
	{ window->set_lost_focus(func); }

#if PLATFORM_WINDOWS
	// Process Window events
	inline void PlatformManager::process_events(MSG msg)
	{
		// Handle all events before updating game
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Change window procedure to new_win_proc
	inline void PlatformManager::change_window_procedure(HWND window_id, u32 index, LONG_PTR new_win_proc)
	{ SetWindowLongPtr(window_id, index, new_win_proc); }
#endif // PLATFORM_WINDOWS
}