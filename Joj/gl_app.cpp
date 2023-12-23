#include "gl_app.h"

#include "engine.h"

void GLApp::init()
{
	
}

void GLApp::update()
{
    // Exit with ESCAPE key
	if (JojEngine::Engine::pm->is_key_pressed(VK_ESCAPE))
		JojEngine::Engine::close_engine();
}

void GLApp::draw()
{
	JojEngine::Engine::gl_graphics->clear();
}

void GLApp::shutdown()
{
	
}