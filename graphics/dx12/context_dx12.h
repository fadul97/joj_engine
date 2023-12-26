#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "graphics_context.h"
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

		IDXGIDebug1* debug;	// D3D12 debug

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context

		ID3D12Device4* get_device();			// Return Graphics device
		IDXGIFactory6* get_factory() const;		// Return DXGI factory interface

	private:
		ID3D12Device4* device;			// Graphics device

		IDXGIFactory6* factory;			// Main DXGI interface


		void log_hardware_info();	// Show hardware information
	};

	// Return Graphics device
	inline ID3D12Device4* DX12Context::get_device()
	{ return device; }

	// Return DXGI factory interface
	inline IDXGIFactory6* DX12Context::get_factory() const
	{ return factory; }
}

#endif // PLATFORM_WINDOWS