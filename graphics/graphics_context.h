#pragma once

#include "defines.h"

#include <memory>
#include "platform_manager.h"

namespace JojGraphics
{
	class GraphicsContext
	{
	public:
		GraphicsContext();
		virtual ~GraphicsContext() = 0;

		virtual b8 init(std::unique_ptr<JojPlatform::Window>& window) = 0;	// Initialize context
	protected:
		virtual void log_hardware_info() = 0;	// Show hardware information
	};
}