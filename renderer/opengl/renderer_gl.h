#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "renderer.h"
#include "opengl/context_gl.h"

namespace JojRenderer
{
	class GLRenderer : public Renderer
	{
	public:
		GLRenderer();
		~GLRenderer();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize renderer
		void render();											// Draw to screen
		void clear();											// Clear screen
		void swap_buffers();									// Change front and back buffers
		void shutdown();										// Clear resources

	private:
		std::unique_ptr<JojGraphics::GLContext> context;

		
	};
}

#endif  // PLATFORM_WINDOWS