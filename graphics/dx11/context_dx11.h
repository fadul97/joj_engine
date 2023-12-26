#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "graphics_context.h"
#include <dxgi1_6.h>	// DirectX graphics infrastructure
#include <d3d11.h>      // Main Direct3D functions
#include <dxgidebug.h>

namespace JojGraphics
{
	class DX11Context : public GraphicsContext
	{
	public:
		DX11Context();
		~DX11Context();

		ID3D11Debug* debug;

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context
	
		ID3D11Device* get_device();			// Return Graphics device
		ID3D11DeviceContext* get_context();	// Return Graphics device context
		IDXGIFactory6* get_factory() const;	// Return DXGI factory interface

	private:
		ID3D11Device* device;			// Graphics device
		ID3D11DeviceContext* context;	// Graphics device context

		IDXGIFactory6* factory;							// Main DXGI interface


		void log_hardware_info();	// Show hardware information
	};

	// Return Graphics device
	inline ID3D11Device* DX11Context::get_device()
	{ return device; }

	// Return Graphics device context
	inline ID3D11DeviceContext* DX11Context::get_context()
	{ return context; }

	// Return DXGI factory interface
	inline IDXGIFactory6* DX11Context::get_factory() const
	{ return factory; }
}

#endif // PLATFORM_WINDOWS