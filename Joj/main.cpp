// main.cpp : Defines the entry point for the application.

#include "window_win32.h"
#include "input_win32.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// Setup and create window
	auto window = new JojPlatform::Win32Window();

	window->set_mode(JojPlatform::WINDOWED);
	window->set_size(1024, 600);
	window->set_color(255, 0, 0);
	window->set_title("Joj Engine");
	window->create();

	auto input = new JojPlatform::Win32Input();

	MSG msg = { 0 };

	u32 x = window->get_xcenter();
	u32 y = window->get_ycenter();

	window->print_on_window("Keycapture Test", x - 55, y - 75, RGB(255, 255, 255));


	// Handle messages destined for the application window
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Close when pressing ESPACE
		if (input->is_key_press(VK_ESCAPE))
			window->close();
	}

	// Cleanup
	delete input;
	delete window;
	
	return 0;
}