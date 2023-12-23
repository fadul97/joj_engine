#include "game.h"

#include "engine.h"

// Static members
JojPlatform::Window* JojEngine::Game::window = nullptr;	// Pointer to window
JojPlatform::Input* JojEngine::Game::input = nullptr;		// Pointer to input

JojEngine::Game::Game()
{
	// FIXME: Refactor attributes
	if (!window)
	{
		//window = JojEngine::Engine::window;
		//input = JojEngine::Engine::input;
	}
}

JojEngine::Game::~Game()
{
}
