#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "context.h"
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

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context
	
		//std::unique_ptr<ID3D11Device> get_device_ownership();					// Pass ownership of ID3D11Device
		//std::unique_ptr<ID3D11DeviceContext> get_device_context_ownsership();	// Pass ownership of ID3D11DeviceContext

		ID3D11Device* get_device();			// Return Graphics device
		ID3D11DeviceContext* get_context();	// Return Graphics device context

		IDXGIFactory6* get_factory() const;				// Return DXGI factory interface

		//IDXGIDebug* debug_dev;
		ID3D11Debug* debug;

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
/*
	// Pass ownership of ID3D11Device
	inline std::unique_ptr<ID3D11Device> DX11Context::get_device_ownership()
	{ return std::move(device); }

	// Pass ownership of ID3D11DeviceContext
	inline std::unique_ptr<ID3D11DeviceContext> DX11Context::get_device_context_ownsership()
	{ return std::move(context); }
*/
	// Return DXGI factory interface
	inline IDXGIFactory6* DX11Context::get_factory() const
	{ return factory; }
}

#endif // PLATFORM_WINDOWS