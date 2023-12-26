#include "context_dx11.h"

#include <sstream>
#include "logger.h"
#include <stdlib.h>

JojGraphics::DX11Context::DX11Context()
{
	device = nullptr;	// Graphics device
	context = nullptr;	// Graphics device context

	factory = nullptr;	// Main DXGI interface

	//#if defined _DEBUG
	debug = nullptr;
	//#endif // _DEBUG
}

JojGraphics::DX11Context::~DX11Context()
{
	// Released in DX11Renderer destructor
	//if (context) context->Release();

	// Released in DX11Renderer destructor
	//if (device) device->Release();

	// Main DXGI interface
	if (factory)
		factory->Release();

	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	//#if defined _DEBUG
	debug->Release();
	//#endif // _DEBUG
}

b8 JojGraphics::DX11Context::init(std::unique_ptr<JojPlatform::Window>& window)
{
	// ---------------------------------------------------
	// Creates DXGI infrastructure and D3D device
	// ---------------------------------------------------

	u32 factory_flags = 0;
	u32 create_device_flags = 0;

	//#if defined _DEBUG
		// Enable DXGI debug layer
	factory_flags = DXGI_CREATE_FACTORY_DEBUG;

	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;

	// TODO: Create(?) D3D11 debug layer
	//ID3D11Debug* debug_controller;
	//debug_controller->EnableDebugLayer();

	//if FAILED(device->QueryInterface(IID_PPV_ARGS(&debug)))
	//{
	//	// TODO: Use own logger
	//	FFATAL(ERR_CONTEXT, "Failed to DXGIGetDebugInterface1.");
	//}

	//debug_dev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
//#endif // _DEBUG

	if FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)))
	{
		FFATAL(ERR_CONTEXT, "Failed to CreateDXGIFactory2.");
		return false;
	}

	// Level of D3D features supported by hardware
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	// Raw pointers
	//ID3D11Device* raw_device = nullptr;
	//ID3D11DeviceContext* raw_context = nullptr;

	if FAILED(D3D11CreateDevice(
		NULL,						// Default adapter
		D3D_DRIVER_TYPE_HARDWARE,	// D3D driver type (Hardware, Reference or Software)
		NULL,						// Pointer to software rasterizer - No software device
		create_device_flags,		// Debug or Normal Mode
		NULL,						// Default feature level (NULL = max supported)
		0,							// Size of feature level array
		D3D11_SDK_VERSION,			// Direct3D SDK version
		&device,				// Stores D3D device created
		&feature_level,				// Current Direct3D version in use,
		&context))			// D3D context device
	{
		// Create software device
		if FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP,
			NULL, create_device_flags, NULL, 0, D3D11_SDK_VERSION,
			&device, &feature_level, &context))
		{
			FWARN("Failed to create device, using WARP Adapter");
		}
	}

#if defined _DEBUG
	if FAILED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug))
	{
		FWARN("Failed to QueryInterface of ID3D11Debug.");
	}
#endif // _DEBUG

	// Reset unique ptrs
	//device.reset(raw_device);
	//context.reset(raw_context);

	// Get pointer to Direct3D device 
	IDXGIDevice* dxgi_device = nullptr;
	if FAILED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device))
	{
		FFATAL(ERR_CONTEXT, "Failed to QueryInterface of device.");
		return false;
	}

	// Get Adpapter from Direct3D device (d3d11Device)
	IDXGIAdapter* dxgi_adapter = nullptr;
	if FAILED(dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgi_adapter))
	{
		FFATAL(ERR_CONTEXT, "Failed to GetParent of dxgi_device.");
		return false;
	}

	// Get pointer to adapter DXGIFactory
	IDXGIFactory2* dxgi_factory = nullptr;
	if FAILED(dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory))
	{
		FFATAL(ERR_CONTEXT, "Failed to GetParent of dxgi_adapter");
		return false;
	}

	// Display graphics hardware information in Visual Studio Output
#ifdef _DEBUG
	log_hardware_info();
#endif 

	// ---------------------------------------------------
	//	Release DXGI Interfaces
	// ---------------------------------------------------

	dxgi_device->Release();
	dxgi_adapter->Release();
	dxgi_factory->Release();
}

void JojGraphics::DX11Context::log_hardware_info()
{
	const u32 bytes_in_megabyte = 1048576U;

	// --------------------------------------
	// Video adapter (Graphics card)
	// --------------------------------------
	IDXGIAdapter* adapter = nullptr;
	if (factory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		char graphics_card[128];
		size_t converted_chars = 0;
		wcstombs_s(&converted_chars, graphics_card, sizeof(graphics_card), desc.Description, _TRUNCATE);
		FINFO("---> Graphics card: %s.", graphics_card);
	}

	IDXGIAdapter4* adapter4 = nullptr;
	if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO mem_info;
		adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &mem_info);

		FINFO("---> Video memory (free): %dMB.", mem_info.Budget / bytes_in_megabyte);
		FINFO("---> Video memory(used) : % dMB.", mem_info.CurrentUsage / bytes_in_megabyte);

		adapter4->Release();
	}

	// -----------------------------------------
	// TODO: Max feature Level supported by GPU
	// -----------------------------------------

	// Instructions block
	{
		FINFO("---> Feature Level: 11_0");
	}

	// -----------------------------------------
	// Video output (monitor)
	// -----------------------------------------

	IDXGIOutput* output = nullptr;
	if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		char device_name[32];
		size_t converted_chars = 0;
		wcstombs_s(&converted_chars, device_name, sizeof(device_name), desc.DeviceName, _TRUNCATE);
		FINFO("---> Monitor: %s.", device_name);
	}

	// ------------------------------------------
	// Video mode (resolution)
	// ------------------------------------------

	// Get screen dimensions
	u32 dpi = GetDpiForSystem();
	u32 screen_width = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
	u32 screen_height = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);

	// Get screen update frequency
	DEVMODE dev_mode = { 0 };
	dev_mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dev_mode);
	u32 refresh = dev_mode.dmDisplayFrequency;

	FINFO("---> Resolution: %dx%d %d Hz.", screen_width, screen_height, refresh);

	// Release used DXGI interfaces
	if (adapter) adapter->Release();
	if (output) output->Release();
}