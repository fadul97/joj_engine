#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "context.h"
#include <dxgi1_6.h>	// DirectX graphics infrastructure
#include <d3d12.h>      // Main Direct3D functions
#include <dxgidebug.h>

namespace JojGraphics
{
	class DX12Context : public GraphicsContext
	{
	public:
		DX12Context();
		~DX12Context();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context

		ID3D12Device4* get_device();			// Return Graphics device

		IDXGIFactory6* get_factory() const;		// Return DXGI factory interface

		//IDXGIDebug* debug_dev;
		ID3D12Debug* debug;

	private:
		ID3D12Device4* device;			// Graphics device

		IDXGIFactory6* factory;			// Main DXGI interface


		void log_hardware_info();	// Show hardware information
		b8 wait_command_queue();	// Wait for command queue execution
		void submit_commands();		// Submit pending commands for execution
	};

	// Return Graphics device
	inline ID3D12Device4* DX12Context::get_device()
	{ return device; }

	// Return DXGI factory interface
	inline IDXGIFactory6* DX12Context::get_factory() const
	{ return factory; }
}

#endif // PLATFORM_WINDOWS