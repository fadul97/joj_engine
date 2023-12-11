#pragma once

#include "game.h"

class MyGame : public JojEngine::Game
{
public:
	void init();
	void update();
	void display();
	void shutdown();
};