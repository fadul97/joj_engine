// main.cpp : Defines the entry point for the application.

#include "window_win32.h"
#include "input_win32.h"
#include <sstream>

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
	std::stringstream text;

	u32 x = window->get_xcenter();
	u32 y = window->get_ycenter();

	// Handle messages destined for the application window
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Close when pressing ESPACE
		if (input->is_key_press(VK_ESCAPE))
			window->close();

		// Mouse position
		text.str("");
		text << input->get_xmouse() << " x " << input->get_ymouse();

		// Mouse buttons
		if (input->is_key_down(VK_LBUTTON))
		{
			window->clear();
			window->print_on_window("BUTTON LEFT", x - 45, y - 230, RGB(255, 255, 255));
			window->print_on_window(text.str().c_str(), x - 32, y - 210, RGB(255, 255, 255));
		}
		if (input->is_key_down(VK_MBUTTON))
		{
			window->clear();
			window->print_on_window("BUTTON MIDDLE", x - 50, y - 230, RGB(255, 255, 255));
			window->print_on_window(text.str().c_str(), x - 32, y - 210, RGB(255, 255, 255));
		}
		if (input->is_key_down(VK_RBUTTON))
		{
			window->clear();
			window->print_on_window("BUTTON RIGHT", x - 45, y - 230, RGB(255, 255, 255));
			window->print_on_window(text.str().c_str(), x - 32, y - 210, RGB(255, 255, 255));
		}

		// Mouse wheel
		if (i16 rotation = input->get_mouse_wheel(); rotation != 0)
		{
			window->clear();
			window->print_on_window("SCROLL", x - 35, y - 250, RGB(255, 255, 255));

			if (rotation > 0)
				window->print_on_window("FORWARD", x - 52, y - 230, RGB(255, 255, 255));
			else
				window->print_on_window("BACKWARD", x - 40, y - 230, RGB(255, 255, 255));

			text.str("");
			text << rotation;
			window->print_on_window(text.str().c_str(), x - 8, y - 210, RGB(255, 255, 255));
		}
	}

	// Cleanup
	delete input;
	delete window;
	
	return 0;
}