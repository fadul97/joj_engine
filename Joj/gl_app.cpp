#include "gl_app.h"

#include "engine.h"

void GLApp::init()
{
	
}

void GLApp::update()
{
    // Exit with ESCAPE key
	if (input->is_key_press(VK_ESCAPE))
		JojEngine::Engine::close_engine();
}

void GLApp::draw()
{
	
}

void GLApp::shutdown()
{
	
}