#include "renderer_dx12.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "error.h"

JojRenderer::DX12Renderer::DX12Renderer()
{
    window = nullptr;

    device = nullptr;
    factory = nullptr;
    swap_chain = nullptr;
    
    command_queue = nullptr;
    command_list = nullptr;
    command_list_alloc = nullptr;

    fence = nullptr;
    current_fence = 0;
}

/* ATTENTION: Call DX12Renderer destructor before DX12Graphics destructor */
JojRenderer::DX12Renderer::~DX12Renderer()
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

b8 JojRenderer::DX12Renderer::init(JojPlatform::Win32Window* window, JojGraphics::DX12Graphics* graphics)
{
    // Set window
    this->window = window;

    // Get objects created from graphics
    device = graphics->get_device();
	factory = graphics->get_factory();

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

    return true;
}

void JojRenderer::DX12Renderer::draw()
{

}

void JojRenderer::DX12Renderer::clear()
{

}

b8 JojRenderer::DX12Renderer::wait_command_queue()
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

void JojRenderer::DX12Renderer::submit_commands()
{
	// submits the commands recorded in the list for execution on the GPU
	command_list->Close();
	ID3D12CommandList* cmds_lists[] = { command_list };
	command_queue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);

	// Wait until GPU completes executing the commands
	wait_command_queue();
}