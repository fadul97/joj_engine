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

		
		if (input->is_key_press(VK_ESCAPE))
			window->close();

		// CTRL, ENTER, ESPAÇO
		if (input->is_key_down(VK_CONTROL))
			window->print_on_window("CTRL", x - 125, y + 150, RGB(255, 255, 255));
		if (input->is_key_down(VK_RETURN))
			window->print_on_window("ENTER", x - 25, y + 150, RGB(255, 255, 255));
		if (input->is_key_down(VK_SPACE))
			window->print_on_window("SPACE", x + 75, y + 150, RGB(255, 255, 255));

		// setas de direção
		if (input->is_key_down(VK_LEFT))
			window->print_on_window("LEFT", x - 45, y, RGB(255, 255, 255));
		if (input->is_key_down(VK_RIGHT))
			window->print_on_window("RIGHT", x, y, RGB(255, 255, 255));
		if (input->is_key_down(VK_UP))
			window->print_on_window("UP", x - 10, y - 20, RGB(255, 255, 255));
		if (input->is_key_down(VK_DOWN))
			window->print_on_window("DOWN", x - 25, y + 20, RGB(255, 255, 255));

		// teclar R
		if (input->is_key_press('R'))
		{
			window->clear();
			window->print_on_window("Keycapture Test", x - 55, y - 75, RGB(255, 255, 255));
		}
		
	}

	// Cleanup
	delete input;
	delete window;
	
	return 0;
}