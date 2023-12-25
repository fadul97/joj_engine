#pragma once

#include "defines.h"

#include <memory>
#include "platform_manager.h"

namespace JojRenderer
{
	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer() = 0;

		virtual b8 init(std::unique_ptr<JojPlatform::Window>& window) = 0;		// Initialize renderer
		virtual void render() = 0;			// Draw to screen
		virtual void clear() = 0;			// Clear screen
		virtual void swap_buffers() = 0;	// Change front and back buffers
		virtual void shutdown() = 0;		// Clear resources
	private:

	};
}