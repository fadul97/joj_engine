#include "game.h"

#include "engine.h"

// Static members
JojPlatform::Win32Window* JojEngine::Game::window = nullptr;	// Pointer to window
JojPlatform::Win32Input* JojEngine::Game::input = nullptr;		// Pointer to input

JojEngine::Game::Game()
{
	if (!window)
	{
		window = JojEngine::Engine::window;
		input = JojEngine::Engine::input;
	}
}

JojEngine::Game::~Game()
{
}
