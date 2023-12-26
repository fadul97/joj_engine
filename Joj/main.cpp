// main.cpp : Defines the entry point for the application.

#include "engine.h"
#include "shapes.h"
#include "app_d3d11.h"
#include "gl_app.h"
#include "error.h"

#if PLATFORM_WINDOWS

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Create a console window
#if _DEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	try
	{
		// Create Engine and setup window
		auto engine = new JojEngine::Engine();

		// Create and execute game
		int exit_code = engine->start(new D3D11App(), JojEngine::RendererBackend::DX11);

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