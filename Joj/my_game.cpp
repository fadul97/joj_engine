#include "my_game.h"

void MyGame::init()
{
	// Get mouse position
	lastX = input->get_xmouse();
	lastY = input->get_ymouse();

	// Game window size end mode
	textSize << "Size: " << window->get_width() << " x " << window->get_height();
	textMode << "Mode: " << (window->get_mode() == JojPlatform::WINDOWED ? "Window" : "Fullscreen");
	textMouse << input->get_xmouse() << " x " << input->get_ymouse();
}

void MyGame::update()
{
	// Exit with ESCAPE key
	if (input->is_key_press(VK_ESCAPE))
		window->close();

	// Converts mouse position to text for display
	textMouse.str("");
	textMouse << input->get_xmouse() << " x " << input->get_ymouse();

	// If mouse changed position
	if (lastX != input->get_xmouse() || lastY != input->get_ymouse())
	{
		window->clear();
		display();
	}

	// Updates mouse position
	lastX = input->get_xmouse();
	lastY = input->get_ymouse();
}

void MyGame::display()
{
	window->print_on_window("MyGame Demo", 10, 10, RGB(255, 255, 255));
	window->print_on_window(textSize.str(), 10, 50, RGB(255, 255, 255));
	window->print_on_window(textMode.str(), 10, 75, RGB(255, 255, 255));
	window->print_on_window(textMouse.str(), 10, 100, RGB(255, 255, 255));
}

void MyGame::shutdown()
{
}