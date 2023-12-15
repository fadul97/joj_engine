#pragma once

#include "game.h"
#include "dx12/renderer_dx12.h"
#include "DirectXMath.h"
#include "geometry.h"

class D3D11Triangle : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

private:
	
};