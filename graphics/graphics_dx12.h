#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <dxgi1_6.h>    // DirectX graphics infrastructure
#include <d3d12.h>      // Main Direct3D functions
#include "window_win32.h"

namespace JojGraphics
{
	class DX12Graphics
	{
	public:
		DX12Graphics();
		~DX12Graphics();

		void init(JojPlatform::Win32Window* window);	// Initialize Direct3D

		ID3D12Device4* get_device();					// Return graphics device
		IDXGIFactory6* get_factory();					// Return DXGI interface


	private:
		// Pipeline
		ID3D12Device4* device;							// Graphics device
		IDXGIFactory6* factory;							// Main DXGI interface

		void log_hardware_info();	// Show hardware information
	};

	// Return graphics device
	inline ID3D12Device4* DX12Graphics::get_device()
	{ return device; }

	// Return DXGI interface
	inline IDXGIFactory6* DX12Graphics::get_factory()
	{ return factory; }
}

#endif  // PLATFORM_WINDOWS