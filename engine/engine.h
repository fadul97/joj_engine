#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "window_win32.h"
#include "input_win32.h"
#include "timer_win32.h"
#include "graphics_dx12.h"
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
		static JojGraphics::DX12Graphics* graphics;	// Graphics device
		static JojEngine::Game* game;				// Game to be executed
		static f32 frametime;						// Current frametime

		i32 start(JojEngine::Game* game);			// Initializes game execution

		static void pause();						// Pause engine
		static void resume();						// Resume engine

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static JojPlatform::Win32Timer timer;	// Time counter
		static b8 paused;						// Engine state

		f32 get_frametime();					// Calculate frametime
		i32 loop();								// Main loop
	};

	inline void Engine::pause()
	{ paused = true; timer.stop(); }

	inline void Engine::resume()
	{ paused = false; timer.start(); }
}