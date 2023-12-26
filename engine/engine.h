#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "platform_manager.h"
#include "dx12/renderer_dx12.h"
#include "dx11/renderer_dx11.h"
#include "opengl/context_gl.h"
#endif	// PLATFORM_WINDOWS

#include <memory>
#include "game.h"

namespace JojEngine
{
	enum class RendererBackend { DX11, DX12, OPENGL };

	std::string renderer_to_string(RendererBackend renderer_backend);

	class Engine
	{
	public:
		Engine();
		~Engine();

		static std::unique_ptr<JojPlatform::PlatformManager> pm;			// Platform Manager
		static std::unique_ptr<JojRenderer::DX11Renderer> renderer;			// DX11 Renderer
		static std::unique_ptr<JojRenderer::DX12Renderer> dx12_renderer;	// DX12 Renderer

		static std::unique_ptr<JojGraphics::GLContext> gl_context;			// OpenGL context


		static JojEngine::Game* game;							// Game to be executed
		static f32 frametime;									// Current frametime

		static std::string renderer_name;	// Hold current Renderer backend name

		i32 start(JojEngine::Game* game, RendererBackend renderer_backend);	// Initializes game execution

		static void close_engine();									// Close Engine

		static void pause();	// Pause engine
		static void resume();	// Resume engine

		// Handle Windows events
		static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		static b8 running;						// Control wether engine is running
		static b8 paused;						// Engine state

		f32 get_frametime();					// Calculate frametime
		i32 loop();								// Main loop
	};

	// Close Engine
	inline void Engine::close_engine()
	{ running = false; }

	inline void Engine::pause()
	{ paused = true; pm->stop_timer(); }

	inline void Engine::resume()
	{ paused = false; pm->start_timer(); }

	inline std::string renderer_to_string(RendererBackend renderer_backend)
	{
		switch (renderer_backend)
		{
		case JojEngine::RendererBackend::DX11:
			return "DirectX 11";
			break;
		case JojEngine::RendererBackend::DX12:
			return "DirectX 12";
			break;
		case JojEngine::RendererBackend::OPENGL:
			return "OpenGL";
			break;
		default:
			return "Default";
			break;
		}
	};
}