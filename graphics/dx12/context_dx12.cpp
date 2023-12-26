#include "context_dx12.h"

#include "context_dx11.h"

#include <sstream>
#include "logger.h"
#include <stdlib.h>

JojGraphics::DX12Context::DX12Context()
{
	device = nullptr;	// Graphics device

	factory = nullptr;	// Main DXGI interface

	//#if defined _DEBUG
	debug = nullptr;
	//#endif // _DEBUG
}

JojGraphics::DX12Context::~DX12Context()
{
	// Released in DX11Renderer destructor
	//if (context) context->Release();

	// Released in DX11Renderer destructor
	//if (device) device->Release();

	// Main DXGI interface
	if (factory)
		factory->Release();

	//#if defined _DEBUG
	//debug->Release();
	//#endif // _DEBUG
}

b8 JojGraphics::DX12Context::init(std::unique_ptr<JojPlatform::Window>& window)
{
	// ---------------------------------------------------
	// Creates DXGI infrastructure and D3D device
	// ---------------------------------------------------

	u32 factory_flags = 0;

#ifdef _DEBUG
	// Enable DXGI debug layer
	factory_flags = DXGI_CREATE_FACTORY_DEBUG;

	// Enable D3D12 debug layer
	ID3D12Debug* debug_controller;
	if FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))
	{
		FWARN("Failed to get DebugInterface of DirectX 12.");
	}

	debug_controller->EnableDebugLayer();
#endif

	if FAILED(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)))
	{
		FFATAL(ERR_RENDERER, "Failed to CreateDXGIFactory2.");
		return false;
	}

	// Create object for graphics device
	if FAILED(D3D12CreateDevice(
		nullptr,                                // Video adapter (nullptr = default adapter)
		D3D_FEATURE_LEVEL_11_0,                 // Minimum version of Direct3D features
		IID_PPV_ARGS(&device)))                 // Save the created D3D device
	{
		// Try to create a WARP device
		IDXGIAdapter* warp;
		if FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)))
		{
			FWARN("Failed to create device, using WARP Adapter");
		}

		// Create D3D object using WARP device
		if FAILED(D3D12CreateDevice(
			warp,                               // WARP video adapter (software)
			D3D_FEATURE_LEVEL_11_0,             // Minimum version of Direct3D features
			IID_PPV_ARGS(&device)))				// Save the created D3D device
		{
			FERROR(ERR_RENDERER, "Failed to create D3D12 Device object using WARP device.");
			
			// Release object no longer needed
			warp->Release();

			return false;
		}

		// Release object no longer needed
		warp->Release();

		// Report use of a WARP device:
		// implements the functionalities of software D3D12 (slow)
		FDEBUG("---> Using WARP Adapter: D3D12 is not supported\n");
	}

	// Display graphics hardware information
#ifdef _DEBUG
	log_hardware_info();
#endif 

	return true;
}

void JojGraphics::DX12Context::log_hardware_info()
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

	D3D_FEATURE_LEVEL feature_levels[9] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels_info;
	feature_levels_info.NumFeatureLevels = 9;
	feature_levels_info.pFeatureLevelsRequested = feature_levels;

	if FAILED(device->CheckFeatureSupport(
		D3D12_FEATURE_FEATURE_LEVELS,
		&feature_levels_info,
		sizeof(feature_levels_info)))
	{

		FWARN("Failed to check feature support for DirectX 12.");
	}

	// Instructions block
	{
		std::wstringstream text;
		text << L"---> Feature Level: ";

		switch (feature_levels_info.MaxSupportedFeatureLevel)
		{
		case D3D_FEATURE_LEVEL_12_1: text << L"12_1\n"; break;
		case D3D_FEATURE_LEVEL_12_0: text << L"12_0\n"; break;
		case D3D_FEATURE_LEVEL_11_1: text << L"11_1\n"; break;
		case D3D_FEATURE_LEVEL_11_0: text << L"11_0\n"; break;
		case D3D_FEATURE_LEVEL_10_1: text << L"10_1\n"; break;
		case D3D_FEATURE_LEVEL_10_0: text << L"10_0\n"; break;
		case D3D_FEATURE_LEVEL_9_3:  text << L"9_3\n";  break;
		case D3D_FEATURE_LEVEL_9_2:  text << L"9_2\n";  break;
		case D3D_FEATURE_LEVEL_9_1:  text << L"9_1\n";  break;
		}

		FINFO("---> Feature Level: %s.", text.str().c_str());
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