#include "my_game.h"

void MyGame::init()
{
}

void MyGame::update()
{
	// Exit with ESCAPE key
	if (input->is_key_press(VK_ESCAPE))
		window->close();
}

void MyGame::display()
{
}

void MyGame::shutdown()
{
}