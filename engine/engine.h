#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "window_win32.h"
#include "input_win32.h"
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

		u32 start(JojEngine::Game* game);	// Initializes game execution

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static JojEngine::Game* game;	// Game to be executed
		u32 loop();						// Main loop
	};
}