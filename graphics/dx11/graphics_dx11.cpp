#include "graphics_dx11.h"

#if PLATFORM_WINDOWS

#include <sstream>
#include "error.h"

JojGraphics::DX11Graphics::DX11Graphics()
{
	device = nullptr;
	factory = nullptr;
	context = nullptr;
	viewport = { 0 };

	swap_chain = nullptr;
	render_target_view = nullptr;
	blend_state = nullptr;
	feature_level = D3D_FEATURE_LEVEL_11_0;

	bg_color[0] = 0.0f;                       // Red
	bg_color[1] = 0.0f;                       // Green
	bg_color[2] = 0.0f;                       // Blue
	bg_color[3] = 0.0f;                       // Alpha (0 = transparent)

	vsync = false;
}

JojGraphics::DX11Graphics::~DX11Graphics()
{
	// Release blend state
	if (blend_state)
		blend_state->Release();

	// Release render target view
	if (render_target_view)
		render_target_view->Release();

	// Release swap chain
	if (swap_chain)
	{
		// Direct3D is unable to close when full screen
		swap_chain->SetFullscreenState(false, NULL);
		swap_chain->Release();
	}

	//  Release graphics device context
	if (context)
	{
		// Restore to original state
		context->ClearState();
		context->Release();
	}

	// Release graphics device
	if (device)
		device->Release();
}

void JojGraphics::DX11Graphics::log_hardware_info()
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
	// TODO: Max feature Level supported by GPU
	// -----------------------------------------
	
	// Instructions block
	{
		std::wstringstream text;
		text << L"---> Feature Level: 11_0\n";
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

void JojGraphics::DX11Graphics::init(JojPlatform::Window* window)
{
	// ---------------------------------------------------
	// Creates DXGI infrastructure and D3D device
	// ---------------------------------------------------

	u32 factory_flags = 0;

#ifdef _DEBUG
	// Enable DXGI debug layer
	factory_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create object for DirectX graphics infrastructure (DXGI)
	ThrowIfFailed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)));

	// Display graphics hardware information in Visual Studio Output
#ifdef _DEBUG
	log_hardware_info();
#endif 

	// Create object to access Direct3D device
	ThrowIfFailed(D3D11CreateDevice(
		NULL,                           // Video adapter (NULL = default adapter)
		D3D_DRIVER_TYPE_HARDWARE,       // D3D driver type (Hardware, Reference or Software)
		NULL,                           // Pointer to rasterizer in software
		factory_flags,					// Debug mode or normal mode
		NULL,                           // Direct3D feature_level (NULL = highest supported)
		0,                              // feature_level vector size
		D3D11_SDK_VERSION,              // Direct3D SDK version
		&device,                        // Save the created D3D device
		&feature_level,                 // Version of Direct3D used
		&context));                     // D3D device context

	// -------------------------------
	// Direct3D background color
	// -------------------------------

	// Adjust backbuffer background color
	// to the same window background color
	COLORREF color = window->get_color();

	bg_color[0] = GetRValue(color) / 255.0f;    // Red
	bg_color[1] = GetGValue(color) / 255.0f;	// Green
	bg_color[2] = GetBValue(color) / 255.0f;	// Blue
	bg_color[3] = 1.0f;							// Alpha (1 = solid)

	// -------------------------------
	// DXGI interfaces
	// -------------------------------

	// Create object for graphics infrastructure
	IDXGIDevice* dxgi_device = nullptr;
	ThrowIfFailed(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device));

	// Create object for video adapter (graphics card)
	IDXGIAdapter* dxgi_adapter = nullptr;
	ThrowIfFailed(dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgi_adapter));

	// Create object for DXGI factory
	IDXGIFactory* dxgi_factory = nullptr;
	ThrowIfFailed(dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgi_factory));

	// -------------------------------
	// Swap Chain 
	// -------------------------------

	 // Description of a swap chain
	DXGI_SWAP_CHAIN_DESC swap_desc = { 0 };
	swap_desc.BufferDesc.Width = u32(window->get_width());					// Backbuffer width
	swap_desc.BufferDesc.Height = u32(window->get_height());				// backbuffer height
	swap_desc.BufferDesc.RefreshRate.Numerator = 60;						// Refresh rate in hertz
	swap_desc.BufferDesc.RefreshRate.Denominator = 1;						// Numerator is an integer and not a rational
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				// 8-bit RGBA color format
	swap_desc.SampleDesc.Count = 1;											// Samples per pixel (antialiasing)
	swap_desc.SampleDesc.Quality = 0;										// Image quality level
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;				// Use surface as RENDER-TARGET
	swap_desc.BufferCount = 2;												// Number of buffers (front + back)
	swap_desc.OutputWindow = window->get_id();								// window ID
	swap_desc.Windowed = (window->get_mode() != JojPlatform::FULLSCREEN);   // Windowed or Fullscreen mode
	swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;					// Discard surface after presentation
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;				// Change monitor resolution to Fullscreen

	// Create swap chain
	ThrowIfFailed(dxgi_factory->CreateSwapChain(device, &swap_desc, &swap_chain));

	// Prevent DXGI from monitoring ALT-ENTER and switching between windowed/fullscreen
	ThrowIfFailed(dxgi_factory->MakeWindowAssociation(window->get_id(), DXGI_MWA_NO_ALT_ENTER));

	// -------------------------------
	// Render Target
	// -------------------------------

	// Get backbuffer surface of a swapchain
	ID3D11Texture2D* backbuffer = nullptr;
	ThrowIfFailed(swap_chain->GetBuffer(0, __uuidof(backbuffer), (void**)(&backbuffer)));

	// Create a render-target view of the backbuffer
	ThrowIfFailed(device->CreateRenderTargetView(backbuffer, NULL, &render_target_view));

	// Bind a render-target to the output-merger stage
	context->OMSetRenderTargets(1, &render_target_view, nullptr);

	// -------------------------------
	// Viewport / Rasterizer
	// -------------------------------

	// Setup viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = f32(window->get_width());
	viewport.Height = f32(window->get_height());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Bind viewport to the Raster Stage
	context->RSSetViewports(1, &viewport);

	// ---------------------------------------------
	// Blend State
	// ---------------------------------------------

	D3D11_BLEND_DESC blend_desc = { };
	blend_desc.AlphaToCoverageEnable = false;                                // Highlight the silhouette of sprites
	blend_desc.IndependentBlendEnable = false;                               // Use the same mix for all render targets
	blend_desc.RenderTarget[0].BlendEnable = true;                           // Enable blending
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;             // Source mixing factor
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;        // Target of RGB mixing is inverted alpha
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;                 // Addition operation in color mixing
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;        // Alpha blend source is the alpha of the pixel shader
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;   // Fate of Alpha mixture is inverted alpha
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;            // Addition operation in color mixing
	blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0F;                 // Components of each pixel that can be overwritten

	// Create blend state
	ThrowIfFailed(device->CreateBlendState(&blend_desc, &blend_state));

	// Connect blend state to the Output-Merger stage
	context->OMSetBlendState(blend_state, nullptr, 0xffffffff);

	// -------------------------------
	// Release DXGI interfaces 
	// -------------------------------

	dxgi_device->Release();
	dxgi_adapter->Release();
	dxgi_factory->Release();
	backbuffer->Release();
}

#endif	// PLATFORM_WINDOWS