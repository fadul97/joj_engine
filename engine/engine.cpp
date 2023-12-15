#include "engine.h"

#include <sstream>

// Static members
JojPlatform::Window* JojEngine::Engine::window = nullptr;				// Game window
JojPlatform::Input* JojEngine::Engine::input = nullptr;				// Input device
JojGraphics::DX12Graphics* JojEngine::Engine::dx12_graphics = nullptr;		// DX12 Graphics device
JojGraphics::DX11Graphics* JojEngine::Engine::dx11_graphics = nullptr;		// DX11 Graphics device
JojEngine::Game* JojEngine::Engine::game = nullptr;							// Pointer to game
f32 JojEngine::Engine::frametime = 0.0f;									// Current frametime
JojPlatform::Timer JojEngine::Engine::timer;							// Time counter
b8 JojEngine::Engine::paused = false;										// Engine state

JojRenderer::DX12Renderer* JojEngine::Engine::renderer = nullptr;			// DX12 Renderer

JojEngine::Engine::Engine()
{
	window = new JojPlatform::Window();
	renderer = new JojRenderer::DX12Renderer();
}

JojEngine::Engine::~Engine()
{
	delete game;

	if (dx11_graphics)
		delete dx11_graphics;
	
	if (dx12_graphics)
		delete dx12_graphics;

	delete renderer;
	
	delete input;
	delete window;
}

i32 JojEngine::Engine::start(JojEngine::Game* game, Renderer renderer_api)
{
	this->game = game;

	// Create window game
	window->create();

	// ATTENTION: input must be initialized after window creation
	input = new JojPlatform::Input();

	// Initialize graphics device
	if (renderer_api == Renderer::DX11)
	{
		dx11_graphics = new JojGraphics::DX11Graphics();
		dx11_graphics->init(window);
	}
	else
	{
		dx12_graphics = new JojGraphics::DX12Graphics();
		dx12_graphics->init(window);
		renderer->init(window, dx12_graphics);
	}

	// Change window procedure to EngineProc
	SetWindowLongPtr(window->get_id(), GWLP_WNDPROC, (LONG_PTR)EngineProc);

	// Adjust sleep resolution to 1 millisecond
	timer.time_begin_period();

	// Run main loop
	i32 exit_code = loop();

	// Return sleep resolution to original value
	timer.time_end_period();

	// Close engine
	return exit_code;
}

i32 JojEngine::Engine::loop()
{
	// Start time counter
	timer.start();

	// Windows messages
	MSG msg = { 0 };

	// Initialize game
	game->init();

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
			if (input->is_key_press('P'))
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

	} while (msg.message != WM_QUIT);

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
	frametime = timer.reset();

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

		text << window->get_title().c_str() << "    "
			<< "FPS: " << frame_count << "    "
			<< "Frametime: " << frametime * 1000 << " (ms)";

		SetWindowText(window->get_id(), text.str().c_str());

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