#include "renderer_gl.h"

#include "logger.h"

JojRenderer::GLRenderer::GLRenderer()
{
    context = std::make_unique<JojGraphics::GLContext>();
}

JojRenderer::GLRenderer::~GLRenderer()
{
   
}

b8 JojRenderer::GLRenderer::init(std::unique_ptr<JojPlatform::Window>& window)
{
    // Initialize OpenGL context
    if (!context->init(window))
    {
        FFATAL(ERR_CONTEXT, "Failed to initialize context.");
        return false;
    }
}

void JojRenderer::GLRenderer::render()
{
    // TODO:
}

void JojRenderer::GLRenderer::clear()
{
}

void JojRenderer::GLRenderer::swap_buffers()
{
}

void JojRenderer::GLRenderer::shutdown()
{
}