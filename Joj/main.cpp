// main.cpp : Defines the entry point for the application.

#include "engine.h"
#include "my_game.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Create Engine and setup window
	auto engine = new JojEngine::Engine();
	engine->window->set_mode(JojPlatform::WINDOWED);
	engine->window->set_size(1024, 600);
	engine->window->set_color(0, 122, 204);
	engine->window->set_title("MyGame window");

	// Create and execute game
	int exitCode = engine->start(new MyGame());

	// Cleanup
	delete engine;

	return 0;
}