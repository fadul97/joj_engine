// main.cpp : Defines the entry point for the application.

#include "window_win32.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Setup and create window
	auto window = new JojPlatform::Win32Window();

	window->set_mode(JojPlatform::WINDOWED);
	window->set_size(1024, 600);
	window->set_color(255, 0, 0);
	window->set_title("Joj Engine");
	window->create();
	
	std::string text = { "Hello, Joj Engine!!!" };
	u32 n = text.length();
	window->print_on_window(text, window->get_xcenter() - n * 2, window->get_ycenter(), RGB(255, 255, 255));

	// Handle messages destined for the application window
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup
	delete window;
	
	return 0;
}