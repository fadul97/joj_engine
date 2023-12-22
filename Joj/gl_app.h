#pragma once

#include "game.h"

class GLApp : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

private:
};