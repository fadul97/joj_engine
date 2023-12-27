#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "graphics_context.h"

namespace JojGraphics
{
	class GLContext : public GraphicsContext
	{
	public:
		GLContext();
		~GLContext();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context
		void swap_buffers();

	private:
		HGLRC rc;		// Rendering context for Win32 Window
		u32 color_bits;
		u32 depth_bits;
		u32 pixel_format_attrib_list[16];
		u32 context_attribs[16];

		u32 gl_version_major;
		u32 gl_version_minor;

		void log_hardware_info();	// Show hardware information
	};
}

#endif // PLATFORM_WINDOWS