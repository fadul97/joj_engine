#include "graphics_dx12.h"

#if PLATFORM_WINDOWS

#include "error.h"
#include <sstream>

JojGraphics::DX12Graphics::DX12Graphics()
{
	// Configuration
	backbuffer_count = 2;	// Double buffering
	antialiasing = 1;		// No antialising
	quality = 0;			// Default quality
	vsync = false;			// No vertical sync
	
	// Background color
	bg_color[0] = 0.0f;		// Red
	bg_color[1] = 0.0f;		// Green
	bg_color[2] = 0.0f;		// Blue
	bg_color[3] = 0.0f;		// Alpha (0 = transparent, 1 = solid)

	// Graphics Infrastructure
	factory = nullptr;
	device = nullptr;
	swapchain = nullptr;
	backbuffer_index = 0;	// 0 is the first

	// Direct3D pipeline
	render_targets = new ID3D12Resource * [backbuffer_count] {nullptr};
	depth_stencil = nullptr;
	render_target_heap = nullptr;
	depth_stencil_heap = nullptr;
	rt_descriptor_size = 0;
	ZeroMemory(&viewport, sizeof(viewport));
	ZeroMemory(&scissor_rect, sizeof(scissor_rect));

	command_queue = nullptr;
	command_list = nullptr;
	command_list_alloc = nullptr;

	// CPU/GPU Synchronization
	fence = nullptr;
	current_fence = 0;
}

JojGraphics::DX12Graphics::~DX12Graphics()
{
	// Wait for GPU to finish queued commands
	wait_command_queue();

	// Release depth stencil buffer
	if (depth_stencil)
		depth_stencil->Release();

	// Release render targets buffers
	if (render_targets)
	{
		for (u32 i = 0; i < backbuffer_count; ++i)
		{
			if (render_targets[i])
				render_targets[i]->Release();
		}
		delete[] render_targets;
	}

	// Release fence
	if (fence)
		fence->Release();

	// Release depth stencil heap
	if (depth_stencil_heap)
		depth_stencil_heap->Release();

	// Release render target heap
	if (render_target_heap)
		render_target_heap->Release();

	// Release swap chain
	if (swapchain)
	{
		// Direct3D is unable to close when in fullscreen
		swapchain->SetFullscreenState(false, NULL);
		swapchain->Release();
	}

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

	// ---------------------------------------------------
	// Create queue, list and command allocator
	// ---------------------------------------------------

	// Create GPU command queue
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));

	// Create command allocator
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&command_list_alloc)));

	// Create command list
	ThrowIfFailed(device->CreateCommandList(
		0,										// Using only one GPU
		D3D12_COMMAND_LIST_TYPE_DIRECT,			// Does not inherit state on the GPU
		command_list_alloc,						// Command allocator
		nullptr,								// Pipeline initial state
		IID_PPV_ARGS(&command_list)));			// Command list object

	// ---------------------------------------------------
	// Create fence to synchronize CPU/GPU
	// ---------------------------------------------------

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	// ---------------------------------------------------
	// Swap Chain
	// ---------------------------------------------------

	// TODO: comment specifications on swapchain_desc
	// Specify swap chain
	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	swapchain_desc.Width = window->get_width();
	swapchain_desc.Height = window->get_height();
	swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_desc.SampleDesc.Count = antialiasing;
	swapchain_desc.SampleDesc.Quality = quality;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = backbuffer_count;
	swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	// Create swap chain
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		command_queue,                          // GPU Command Queue
		window->get_id(),                       // Window ID
		&swapchain_desc,                        // Swap chain descriptor
		nullptr,                                // Fullscreen Swap Chain
		nullptr,                                // Restrict output screen
		&swapchain));                           // Swap chain object

	// ---------------------------------------------------
	// Render Target Views (and associated heaps)
	// ---------------------------------------------------

	// TODO: comment specifications on render_target_heap_desc
	// Specify heap for Render Target (RT) descriptor
	D3D12_DESCRIPTOR_HEAP_DESC render_target_heap_desc = {};
	render_target_heap_desc.NumDescriptors = backbuffer_count;
	render_target_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	render_target_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create heap for render target descriptor
	ThrowIfFailed(device->CreateDescriptorHeap(&render_target_heap_desc, IID_PPV_ARGS(&render_target_heap)));

	// Get a Handle to the start of the Heap
	D3D12_CPU_DESCRIPTOR_HANDLE rt_handle = render_target_heap->GetCPUDescriptorHandleForHeapStart();

	/* Value to increment to access the next descriptor within the Heap
	   The size of a descriptor depends on the graphics hardware and the type of heap used */
	rt_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create a Render Target descriptor (view) for each buffer (front and back buffers)
	for (u32 i = 0; i < backbuffer_count; ++i)
	{
		swapchain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i]));
		device->CreateRenderTargetView(render_targets[i], nullptr, rt_handle);
		rt_handle.ptr += rt_descriptor_size;
	}

	// ---------------------------------------------------
	// Depth/Stencil View (e associated heaps)
	// ---------------------------------------------------

	// TODO: comment specifications on depth_stencil_desc
	// Specify Depth/Stencil buffer
	D3D12_RESOURCE_DESC depth_stencil_desc = {};
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = window->get_width();
	depth_stencil_desc.Height = window->get_height();
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_desc.SampleDesc.Count = antialiasing;
	depth_stencil_desc.SampleDesc.Quality = quality;
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// TODO: comment specifications on ds_heap_properties
	// Depth/Stencil buffer heap properties
	D3D12_HEAP_PROPERTIES ds_heap_properties = {};
	ds_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	ds_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	ds_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	ds_heap_properties.CreationNodeMask = 1;
	ds_heap_properties.VisibleNodeMask = 1;

	// TODO: comment specifications on optmized_clear
	// Describe values for clearing the Depth/Stencil buffer
	D3D12_CLEAR_VALUE optmized_clear = {};
	optmized_clear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optmized_clear.DepthStencil.Depth = 1.0f;
	optmized_clear.DepthStencil.Stencil = 0;

	// Create buffer Depth/Stencil
	ThrowIfFailed(device->CreateCommittedResource(
		&ds_heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&depth_stencil_desc,
		D3D12_RESOURCE_STATE_COMMON,
		&optmized_clear,
		IID_PPV_ARGS(&depth_stencil)));

	// TODO: comment specifications on depthstencil_heap_desc
	// Specify Depth/Stencil heap descriptor
	D3D12_DESCRIPTOR_HEAP_DESC depth_stencil_heap_desc = {};
	depth_stencil_heap_desc.NumDescriptors = 1;
	depth_stencil_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depth_stencil_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create heap for Depth/Stencil descriptor
	ThrowIfFailed(device->CreateDescriptorHeap(&depth_stencil_heap_desc, IID_PPV_ARGS(&depth_stencil_heap)));

	// Get a Handle to the start of the Heap
	D3D12_CPU_DESCRIPTOR_HANDLE ds_handle = depth_stencil_heap->GetCPUDescriptorHandleForHeapStart();

	// creates a Depth/Stencil descriptor (view) for mip level 0
	device->CreateDepthStencilView(depth_stencil, nullptr, ds_handle);

	// TODO: comment specifications on barrier
	// Transition from the initial state of the resource to be used as a depth buffer
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = depth_stencil;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->ResourceBarrier(1, &barrier);

	// Submit command to depth/stencil buffer transition
	submit_commands();

	// ---------------------------------------------------
	// Viewport e Scissor Rect
	// ---------------------------------------------------

	// Setup viewport
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(window->get_width());
	viewport.Height = static_cast<float>(window->get_height());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Adjust Scissor Rect
	scissor_rect = { 0, 0, window->get_width(), window->get_height() };

	// ---------------------------------------------------
	// Backbuffer background color
	// ---------------------------------------------------

	// Background color of the backbuffer = window background color
	COLORREF color = window->get_color();

	bg_color[0] = GetRValue(color) / 255.0f;	// Red
	bg_color[1] = GetGValue(color) / 255.0f;	// Green
	bg_color[2] = GetBValue(color) / 255.0f;	// Blue
	bg_color[3] = 1.0f;							// Alpha (1 = solid)
}

b8 JojGraphics::DX12Graphics::wait_command_queue()
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

void JojGraphics::DX12Graphics::submit_commands()
{
	// submits the commands recorded in the list for execution on the GPU
	command_list->Close();
	ID3D12CommandList* cmds_lists[] = { command_list };
	command_queue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);

	// Wait until GPU completes executing the commands
	wait_command_queue();
}

#endif  // PLATFORM_WINDOWS