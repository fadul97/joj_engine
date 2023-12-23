#include "engine.h"

#include <sstream>

// Static members
std::unique_ptr <JojPlatform::PlatformManager> JojEngine::Engine::pm = nullptr;	// Platform Manager

JojGraphics::DX12Graphics* JojEngine::Engine::dx12_graphics = nullptr;		// DX12 Graphics device
JojGraphics::DX11Graphics* JojEngine::Engine::dx11_graphics = nullptr;		// DX11 Graphics device
JojGraphics::GLGraphics* JojEngine::Engine::gl_graphics = nullptr;			// Opengl Graphics device
JojEngine::Game* JojEngine::Engine::game = nullptr;							// Pointer to game
f32 JojEngine::Engine::frametime = 0.0f;									// Current frametime
JojPlatform::Timer JojEngine::Engine::timer;								// Time counter
b8 JojEngine::Engine::paused = false;										// Engine state
b8 JojEngine::Engine::running = false;										// Engine is not running

JojRenderer::DX12Renderer* JojEngine::Engine::renderer = nullptr;			// DX12 Renderer
JojRenderer::DX11Renderer* JojEngine::Engine::dx11_renderer = nullptr;		// DX12 Renderer

JojEngine::Engine::Engine()
{
	pm = std::make_unique<JojPlatform::PlatformManager>();
	renderer = new JojRenderer::DX12Renderer();
	dx11_renderer = new JojRenderer::DX11Renderer();
}

JojEngine::Engine::~Engine()
{
	delete game;

	if (dx11_graphics)
		delete dx11_graphics;
	
	if (dx12_graphics)
		delete dx12_graphics;

	delete renderer;
	delete dx11_renderer;
}

i32 JojEngine::Engine::start(JojEngine::Game* game, Renderer renderer_api)
{
	this->game = game;

	pm->init(800, 600);

	// Initialize graphics device
	if (renderer_api == Renderer::DX11)
	{
		dx11_graphics = new JojGraphics::DX11Graphics();
		dx11_graphics->init(pm->get_window());
		dx11_renderer->init(pm->get_window(), dx11_graphics);
	}
	else if (renderer_api == Renderer::DX12)
	{
		dx12_graphics = new JojGraphics::DX12Graphics();
		dx12_graphics->init(pm->get_window());
		renderer->init(pm->get_window(), dx12_graphics);
	}
	else
	{
		gl_graphics = new JojGraphics::GLGraphics();
		if (!gl_graphics->init(pm->get_window()))
			OutputDebugString("Failed to initialize OpenGL\n");
	}

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