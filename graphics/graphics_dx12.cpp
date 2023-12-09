#include "graphics_dx12.h"

#if PLATFORM_WINDOWS

#include "error.h"
#include <sstream>

JojGraphics::Dx12Graphics::Dx12Graphics()
{
	factory = nullptr;
	device = nullptr;
	command_queue = nullptr;
	command_list = nullptr;
	command_list_alloc = nullptr;
	fence = nullptr;
	current_fence = 0;
}

JojGraphics::Dx12Graphics::~Dx12Graphics()
{
	// Wait for GPU to finish queued commands
	wait_command_queue();

	// Release fence
	if (fence)
		fence->Release();

	// Release command list
	if (command_list)
		command_list->Release();

	// Release command allocator
	if (command_list_alloc)
		command_list_alloc->Release();

	// Release command queue
	if (command_queue)
		command_queue->Release();

	// Release graphics device
	if (device)
		device->Release();

	// Release main interface
	if (factory)
		factory->Release();
}

void JojGraphics::Dx12Graphics::log_hardware_info()
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

void JojGraphics::Dx12Graphics::init(JojPlatform::Win32Window* window)
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
	//ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
	debug_controller->EnableDebugLayer();
#endif

	// Create object for DirectX graphics infrastructure (DXGI)
	//ThrowIfFailed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

	// Create object for graphics device
	if FAILED(D3D12CreateDevice(
		nullptr,                                // Video adapter (nullptr = default adapter)
		D3D_FEATURE_LEVEL_11_0,                 // Minimum version of Direct3D features
		IID_PPV_ARGS(&device)))                 // Save the created D3D device
	{
		// Try to create a WARP device
		IDXGIAdapter* warp;
		//ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)));

		// Create D3D object using WARP device
		//ThrowIfFailed(D3D12CreateDevice(
			//warp,                               // WARP video adapter (software)
			//D3D_FEATURE_LEVEL_11_0,             // Minimum version of Direct3D features
			//IID_PPV_ARGS(&device)));            // Save the created D3D device

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

	// ---------------------------------------------------
	// Create queue, list and command allocator
	// ---------------------------------------------------

	// Create GPU command queue
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//ThrowIfFailed(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));

	// Create command allocator
	//ThrowIfFailed(device->CreateCommandAllocator(
		//D3D12_COMMAND_LIST_TYPE_DIRECT,
		//IID_PPV_ARGS(&command_list_alloc)));

	// Create command list
	//ThrowIfFailed(device->CreateCommandList(
		//0,										// Using only one GPU
		//D3D12_COMMAND_LIST_TYPE_DIRECT,			// Does not inherit state on the GPU
		//command_list_alloc,						// Command allocator
		//nullptr,								// Pipeline initial state
		//IID_PPV_ARGS(&command_list)));			// Command list object

	// ---------------------------------------------------
	// Create fence to synchronize CPU/GPU
	// ---------------------------------------------------

	//ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

}

b8 JojGraphics::Dx12Graphics::wait_command_queue()
{
	// Advance fence value to mark new commands from that point
	current_fence++;

	// Add an instruction to the command queue to insert a new fence
	// GPU will finish all ongoing commands before processing this signal
	if (FAILED(command_queue->Signal(fence, current_fence)))
		return false;

	// Wait for GPU to complete all previous commands
	if (fence->GetCompletedValue() < current_fence)
	{
		HANDLE event_handle = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

		if (event_handle)
		{
			// Trigger event when GPU reaches current fence
			if (FAILED(fence->SetEventOnCompletion(current_fence, event_handle)))
				return false;

			// Wait until the event is triggered
			WaitForSingleObject(event_handle, INFINITE);
			CloseHandle(event_handle);
		}
	}

	return true;
}

void JojGraphics::Dx12Graphics::submit_commands()
{
	// submits the commands recorded in the list for execution on the GPU
	command_list->Close();
	ID3D12CommandList* cmds_lists[] = { command_list };
	command_queue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);

	// Wait until GPU completes executing the commands
	wait_command_queue();
}

#endif  // PLATFORM_WINDOWS