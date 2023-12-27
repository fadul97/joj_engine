// main.cpp : Defines the entry point for the application.

#include "engine.h"
#include "shapes.h"
#include "app_d3d11.h"
#include "gl_app.h"
#include "error.h"

#include "logger.h"

#include <iostream>

#if PLATFORM_WINDOWS

int main()
{
	// Create a console window
#if _DEBUG
	JojPlatform::PlatformManager::create_console();
#endif

	try
	{
		// Create Engine and setup window
		auto engine = new JojEngine::Engine();

		// Create and execute game
		int exit_code = engine->start(new GLApp(), JojEngine::RendererBackend::OPENGL);

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

#endif	// PLATFORM_WINDOWS