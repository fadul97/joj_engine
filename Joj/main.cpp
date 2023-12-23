// main.cpp : Defines the entry point for the application.

#include "engine.h"
#include "shapes.h"
#include "app_d3d11.h"
#include "gl_app.h"
#include "error.h"

#if PLATFORM_WINDOWS

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	try
	{
		// Create Engine and setup window
		auto engine = new JojEngine::Engine();

		// Create and execute game
		int exit_code = engine->start(new Shapes(), JojEngine::Renderer::DX12);

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