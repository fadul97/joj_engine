#pragma once

#include "game.h"

#include <sstream>

class MyGame : public JojEngine::Game
{
public:
	void init();
	void update();
	void display();
	void shutdown();

private:
	std::stringstream textSize;
	std::stringstream textMode;
	std::stringstream textMouse;

	int lastX, lastY;
};