#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "window_win32.h"
#include "input_win32.h"
#include "timer_win32.h"
#endif	// PLATFORM_WINDOWS

#include "game.h"

namespace JojEngine
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		static JojPlatform::Win32Window* window;
		static JojPlatform::Win32Input* input;
		static JojEngine::Game* game;				// Game to be executed
		static f32 frametime;						// Current frametime

		i32 start(JojEngine::Game* game);	// Initializes game execution

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static JojPlatform::Win32Timer timer;	// Time counter
		f32 get_frametime();					// Calculate frametime
		i32 loop();								// Main loop
	};
}