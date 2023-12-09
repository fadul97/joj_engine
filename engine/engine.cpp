#include "engine.h"

// Static members
JojPlatform::Win32Window* JojEngine::Engine::window = nullptr;		// Game window
JojPlatform::Win32Input* JojEngine::Engine::input = nullptr;		// Input device
JojEngine::Game* JojEngine::Engine::game = nullptr;				    // Pointer to game

JojEngine::Engine::Engine()
{
	window = new JojPlatform::Win32Window();
}

JojEngine::Engine::~Engine()
{
	delete game;
	delete input;
	delete window;
}

u32 JojEngine::Engine::start(JojEngine::Game* game)
{
	this->game = game;

	// Create window game
	window->create();

	// ATTENTION: input must be initialized after window creation
	input = new JojPlatform::Win32Input();

	// Change window procedure to EngineProc
	SetWindowLongPtr(window->get_id(), GWLP_WNDPROC, (LONG_PTR)EngineProc);

	// return execution result
	return loop();
}

u32 JojEngine::Engine::loop()
{
	// Windows messages
	MSG msg = { 0 };

	// inicialização da aplicação
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
			// Update game
			game->update();

			// Game draw
			game->draw();

			// Waits 16 milliseconds or the next interaction with the window
			MsgWaitForMultipleObjects(0, NULL, FALSE, 10, QS_ALLINPUT);
		}

	} while (msg.message != WM_QUIT);

	// Shutdown game
	game->shutdown();

	// Close game
	return u32(msg.wParam);
}

LRESULT CALLBACK JojEngine::Engine::EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Window must be repainted
	if (msg == WM_PAINT)
		game->display();

	return CallWindowProc(JojPlatform::Win32Input::InputProc, hWnd, msg, wParam, lParam);
}