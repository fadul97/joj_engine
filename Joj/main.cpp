// main.cpp : Defines the entry point for the application.

#include "engine.h"
#include "my_game.h"
#include "error.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	try
	{
		// Create Engine and setup window
		auto engine = new JojEngine::Engine();
		engine->window->set_mode(JojPlatform::WINDOWED);
		engine->window->set_size(1024, 600);
		engine->window->set_color(0, 122, 204);
		engine->window->set_title("MyGame window");

		// Game pauses/resumes when losing/gaining focus
		engine->window->set_lost_focus(JojEngine::Engine::pause);
		engine->window->set_on_focus(JojEngine::Engine::resume);

		// Create and execute game
		int exit_code = engine->start(new MyGame(), JojEngine::Renderer::DX11);

		// Cleanup
		delete engine;

		// Exit program
		return exit_code;
	}
	catch (JojEngine::Error& e)
	{
		MessageBox(nullptr, e.to_string().data(), "MyGame", MB_OK);
		return 0;
	}
}