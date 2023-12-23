#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "platform_manager.h"
#include "dx12/graphics_dx12.h"
#include "dx11/graphics_dx11.h"
#include "dx12/renderer_dx12.h"
#include "dx11/renderer_dx11.h"
#include "opengl/graphics_gl.h"
#endif	// PLATFORM_WINDOWS

#include <memory>
#include "game.h"

namespace JojEngine
{
	enum class Renderer { DX11, DX12, OPENGL };

	class Engine
	{
	public:
		Engine();
		~Engine();

		static std::unique_ptr<JojPlatform::PlatformManager> pm;	// Platform Manager

		static JojGraphics::DX12Graphics* dx12_graphics;		// DX12 Graphics device
		static JojGraphics::DX11Graphics* dx11_graphics;		// DX11 Graphics device
		static JojGraphics::GLGraphics* gl_graphics;			// OpenGL Graphics device
		static JojEngine::Game* game;							// Game to be executed
		static f32 frametime;									// Current frametime

		static JojRenderer::DX12Renderer* renderer;				// DX12 Renderer
		static JojRenderer::DX11Renderer* dx11_renderer;		// DX11 Renderer

		i32 start(JojEngine::Game* game, Renderer renderer_api);	// Initializes game execution

		static void close_engine();									// Close Engine

		static void pause();	// Pause engine
		static void resume();	// Resume engine

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static b8 running;						// Control wether engine is running
		static JojPlatform::Timer timer;		// Time counter
		static b8 paused;						// Engine state

		f32 get_frametime();					// Calculate frametime
		i32 loop();								// Main loop
	};

	// Close Engine
	inline void Engine::close_engine()
	{ running = false; }

	inline void Engine::pause()
	{ paused = true; timer.stop(); }

	inline void Engine::resume()
	{ paused = false; timer.start(); }
}