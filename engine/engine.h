#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "window.h"
#include "input.h"
#include "timer.h"
#include "dx12/graphics_dx12.h"
#include "dx11/graphics_dx11.h"
#include "dx12/renderer_dx12.h"
#endif	// PLATFORM_WINDOWS

#include "game.h"

namespace JojEngine
{
	enum class Renderer { DX11, DX12 };

	class Engine
	{
	public:
		Engine();
		~Engine();

		static JojPlatform::Window* window;
		static JojPlatform::Input* input;
		static JojGraphics::DX12Graphics* dx12_graphics;		// DX12 Graphics device
		static JojGraphics::DX11Graphics* dx11_graphics;		// DX11 Graphics device
		static JojEngine::Game* game;							// Game to be executed
		static f32 frametime;									// Current frametime

		static JojRenderer::DX12Renderer* renderer;				// DX12 Renderer

		i32 start(JojEngine::Game* game, Renderer renderer_api);	// Initializes game execution

		static void pause();	// Pause engine
		static void resume();	// Resume engine

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static JojPlatform::Timer timer;	// Time counter
		static b8 paused;						// Engine state

		f32 get_frametime();					// Calculate frametime
		i32 loop();								// Main loop
	};

	inline void Engine::pause()
	{ paused = true; timer.stop(); }

	inline void Engine::resume()
	{ paused = false; timer.start(); }
}