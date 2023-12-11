#include "graphics_dx12.h"

#if PLATFORM_WINDOWS

#include "error.h"
#include <sstream>

JojGraphics::DX12Graphics::DX12Graphics()
{
	factory = nullptr;
	device = nullptr;
}

JojGraphics::DX12Graphics::~DX12Graphics()
{
	// Release graphics device
	if (device)
		device->Release();

	// Release main interface
	if (factory)
		factory->Release();
}

void JojGraphics::DX12Graphics::log_hardware_info()
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

		std::wstringstream text;
		text << L"---> Graphics card: " << desc.Description << L"\n";
		OutputDebugStringW(text.str().c_str());
	}

	IDXGIAdapter4* adapter4 = nullptr;
	if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter4))))
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO mem_info;
		adapter4->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &mem_info);

		std::wstringstream text;
		text << L"---> Video memory (free): " << mem_info.Budget / bytes_in_megabyte << L"MB\n";
		text << L"---> Video memory (used): " << mem_info.CurrentUsage / bytes_in_megabyte << L"MB\n";
		OutputDebugStringW(text.str().c_str());

		adapter4->Release();
	}

	// -----------------------------------------
	// Max feature Level supported by GPU
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

	device->CheckFeatureSupport(
		D3D12_FEATURE_FEATURE_LEVELS,
		&feature_levels_info,
		sizeof(feature_levels_info));

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
		OutputDebugStringW(text.str().c_str());
	}

	// -----------------------------------------
	// Video output (monitor)
	// -----------------------------------------

	IDXGIOutput* output = nullptr;
	if (adapter->EnumOutputs(0, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstringstream text;
		text << L"---> Monitor: " << desc.DeviceName << L"\n";
		OutputDebugStringW(text.str().c_str());
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

	std::wstringstream text;
	text << L"---> Resolution: " << screen_width << L"x" << screen_height << L" " << refresh << L" Hz\n";
	OutputDebugStringW(text.str().c_str());

	// ------------------------------------------

	// Release used DXGI interfaces
	if (adapter) adapter->Release();
	if (output) output->Release();
}

void JojGraphics::DX12Graphics::init(JojPlatform::Win32Window* window)
{
	// ---------------------------------------------------
	// Create DXGI infrastructure and D3D device
	// ---------------------------------------------------

	u32 factory_flags = 0;

#ifdef _DEBUG
	// Enable DXGI debug layer
	factory_flags = DXGI_CREATE_FACTORY_DEBUG;

	// Enable D3D12 debug layer
	ID3D12Debug* debug_controller;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
	debug_controller->EnableDebugLayer();
#endif

	// Create object for DirectX graphics infrastructure (DXGI)
	ThrowIfFailed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

	// Create object for graphics device
	if FAILED(D3D12CreateDevice(
		nullptr,                                // Video adapter (nullptr = default adapter)
		D3D_FEATURE_LEVEL_11_0,                 // Minimum version of Direct3D features
		IID_PPV_ARGS(&device)))                 // Save the created D3D device
	{
		// Try to create a WARP device
		IDXGIAdapter* warp;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)));

		// Create D3D object using WARP device
		ThrowIfFailed(D3D12CreateDevice(
			warp,                               // WARP video adapter (software)
			D3D_FEATURE_LEVEL_11_0,             // Minimum version of Direct3D features
			IID_PPV_ARGS(&device)));            // Save the created D3D device

		// Release object no longer needed
		warp->Release();

		// Report use of a WARP device:
		// implements the functionalities of software D3D12 (slow)
		OutputDebugString("---> Using WARP Adapter: D3D12 is not supported\n");
	}

	// Display graphics hardware information in Visual Studio Output
#ifdef _DEBUG
	log_hardware_info();
#endif 
}

#endif  // PLATFORM_WINDOWS