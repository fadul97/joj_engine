#include "engine.h"

#include <sstream>
#include "logger.h"

// Static members
std::unique_ptr<JojPlatform::PlatformManager> JojEngine::Engine::pm = nullptr;		// Platform Manager
std::unique_ptr<JojRenderer::DX11Renderer> JojEngine::Engine::renderer = nullptr;	// Renderer
std::unique_ptr<JojRenderer::DX12Renderer> JojEngine::Engine::dx12_renderer = nullptr;	// Renderer

JojGraphics::GLGraphics* JojEngine::Engine::gl_graphics = nullptr;			// Opengl Graphics device

JojEngine::Game* JojEngine::Engine::game = nullptr;							// Pointer to game
f32 JojEngine::Engine::frametime = 0.0f;									// Current frametime
b8 JojEngine::Engine::paused = false;										// Engine state
b8 JojEngine::Engine::running = false;										// Engine is not running

std::string JojEngine::Engine::renderer_name = "";

JojEngine::Engine::Engine()
{
	pm = std::make_unique<JojPlatform::PlatformManager>();
}

JojEngine::Engine::~Engine()
{
	delete game;
}

i32 JojEngine::Engine::start(JojEngine::Game* game, RendererBackend renderer_backend)
{
	this->game = game;

	if (!pm->init(800, 600))
	{
		// TODO: Use own logger
		OutputDebugString("Failed to initialize platform manager.\n");
		return -1;
	}

	// Initialize graphics device
	if (renderer_backend == RendererBackend::DX11)
	{
		renderer = std::make_unique<JojRenderer::DX11Renderer>();
		if (!renderer->init(pm->get_window()))
		{
			// TODO: Use own logger
			OutputDebugString("Failed to initialize Renderer::DX11.\n");
			return -1;
		}
	}
	else if (renderer_backend == RendererBackend::DX12)
	{
		dx12_renderer = std::make_unique<JojRenderer::DX12Renderer>();
		if (!dx12_renderer->init(pm->get_window()))
		{
			// TODO: Use own logger
			FFATAL(ERR_RENDERER, "Failed to initialize D3D12 renderer.");
			OutputDebugString("Failed to initialize Renderer::DX12.\n");
			return -1;
		}
	}
	else
	{
		gl_graphics = new JojGraphics::GLGraphics();
		if (!gl_graphics->init(pm->get_window()))
		{
			// TODO: Use own logger
			OutputDebugString("Failed to initialize OpenGL\n");
			return -1;
		}
	}

	renderer_name = renderer_to_string(renderer_backend);

	// Change window procedure to EngineProc
	pm->change_window_procedure(pm->get_window()->get_id(), GWLP_WNDPROC, (LONG_PTR)EngineProc);

	// Adjust sleep resolution to 1 millisecond
	pm->begin_period();

	// Game pauses/resumes when losing/gaining focus
	pm->set_lost_focus(JojEngine::Engine::pause);
	pm->set_on_focus(JojEngine::Engine::resume);

	// Run main loop
	i32 exit_code = loop();

	// Return sleep resolution to original value
	pm->end_period();

	// Close engine
	return exit_code;
}

i32 JojEngine::Engine::loop()
{
	// Start time counter
	pm->start_timer();

	// Windows messages
	MSG msg = { 0 };

	// Initialize game
	game->init();

	// Engine is running
	running = true;

	// Main loop
	do
	{
		// Handle all events before updating game
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// -----------------------------------------------
			// Pause/Resume Game
			// -----------------------------------------------
			// P key pauses engine
			if (pm->is_key_pressed('P'))
			{
				if (paused)
					resume();
				else
					pause();
			}

			if (!paused)
			{
				// Calculate frametime
				frametime = get_frametime();

				// Update game
				game->update();

				// Game draw
				game->draw();
			}
			else
			{
				// Game paused
				game->on_pause();
			}
		}

	} while (running && msg.message != WM_QUIT);

	pm->shutdown();

	// Shutdown game
	game->shutdown();

	// Close game
	return i32(msg.wParam);
}

f32 JojEngine::Engine::get_frametime()
{

#ifdef _DEBUG
	static f32 total_time = 0.0f;	// Total time elapsed
	static u32  frame_count = 0;	// Elapsed frame counter
#endif

	// Current frame time
	frametime = pm->reset_timer();

#ifdef _DEBUG
	// Accumulated frametime
	total_time += frametime;

	// Increment frame counter
	frame_count++;

	// Updates FPS indicator in the window every 1000ms (1 second)
	if (total_time >= 1.0f)
	{
		std::stringstream text;		// Text flow for messages
		text << std::fixed;			// Always show the fractional part
		text.precision(3);			// three numbers after comma

		text << pm->get_window()->get_title().c_str() << "    "
			<< "Renderer Backend: " << renderer_name.c_str() << "    "
			<< "FPS: " << frame_count << "    "
			<< "Frametime: " << frametime * 1000 << " (ms)";

		SetWindowText(pm->get_window()->get_id(), text.str().c_str());

		frame_count = 0;
		total_time -= 1.0f;
	}
#endif

	return frametime;
}


LRESULT CALLBACK JojEngine::Engine::EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Window must be repainted
	if (msg == WM_PAINT)
		game->display();

	return CallWindowProc(JojPlatform::Input::InputProc, hWnd, msg, wParam, lParam);
}