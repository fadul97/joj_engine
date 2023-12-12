#include "renderer_dx12.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "error.h"

#include <sstream>

JojRenderer::DX12Renderer::DX12Renderer()
{
    window = nullptr;
    graphics = nullptr;

    // Configuration
    backbuffer_count = 2;	// Double buffering
    vsync = false;			// No vertical sync

    // Background color
    bg_color[0] = 0.0f;		// Red
    bg_color[1] = 0.0f;		// Green
    bg_color[2] = 0.0f;		// Blue
    bg_color[3] = 0.0f;		// Alpha (0 = transparent, 1 = solid)

    // Graphics Infrastructure
    device = nullptr;
    factory = nullptr;
    swapchain = nullptr;
    backbuffer_index = 0;   // 0 is the first

    // Pipeline
    render_targets = nullptr;
    depth_stencil = nullptr;
    render_target_heap = nullptr;
    depth_stencil_heap = nullptr;
    rt_descriptor_size = 0;
    viewport = { 0 };
    scissor_rect = { 0 };

    command_queue = nullptr;
    command_list = nullptr;
    command_list_alloc = nullptr;

    // CPU/GPU Synchronization
    fence = nullptr;
    current_fence = 0;
}

/* ATTENTION: Call DX12Renderer destructor before DX12Graphics destructor */
JojRenderer::DX12Renderer::~DX12Renderer()
{
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
    this->graphics = graphics;

    // Get objects created from graphics
    backbuffer_count = graphics->get_backbuffer_count();
    vsync = graphics->get_vsync();
    device = graphics->get_device();
	factory = graphics->get_factory();
    swapchain = graphics->get_swapchain();
    command_queue = graphics->get_command_queue();
    command_list = graphics->get_command_list();
    command_list_alloc = graphics->get_command_list_alloc();
    render_targets = graphics->get_render_targets();
    depth_stencil = graphics->get_depth_stencil();
    render_target_heap = graphics->get_render_target_heap();
    depth_stencil_heap = graphics->get_depth_stencil_heap();
    viewport = graphics->get_viewport();
    scissor_rect = graphics->get_scissor_rect();

    // Background color of the backbuffer = window background color
    COLORREF color = window->get_color();

    bg_color[0] = GetRValue(color) / 255.0f;	// Red
    bg_color[1] = GetGValue(color) / 255.0f;	// Green
    bg_color[2] = GetBValue(color) / 255.0f;	// Blue
    bg_color[3] = 1.0f;							// Alpha (1 = solid)
    
    rt_descriptor_size = graphics->get_rt_descriptor_size();
    fence = graphics->get_fence();
    current_fence = graphics->get_current_fence();

    return true;
}

void JojRenderer::DX12Renderer::draw()
{

}

void JojRenderer::DX12Renderer::present()
{
    // indica que o backbuffer será usado para apresentação
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = render_targets[backbuffer_index];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &barrier);

    // submete a lista de comandos para execução na GPU
    submit_commands();

    // Show frame and swap front/back buffers
    swapchain->Present(vsync, 0);
    backbuffer_index = (backbuffer_index + 1) % backbuffer_count;
}

void JojRenderer::DX12Renderer::clear(ID3D12PipelineState* pso)
{
    /* Reuses the memory associated with the command list
       The list of commands should have finished running on the GPU */
    command_list_alloc->Reset();

    /* A list of commands can be reinitialized after added to 
     GPU command queue (via ExecuteCommandList) */
    command_list->Reset(command_list_alloc, pso);

    // TODO: comment specifications on barrier
    // Indicate that the backbuffer will be used as a render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = render_targets[backbuffer_index];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &barrier);

    // Adjust viewport and clipping rectangles
    command_list->RSSetViewports(1, &viewport);
    command_list->RSSetScissorRects(1, &scissor_rect);

    // Clear backbuffer and depth/stencil buffer
    D3D12_CPU_DESCRIPTOR_HANDLE ds_handle = depth_stencil_heap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE rt_handle = render_target_heap->GetCPUDescriptorHandleForHeapStart();
    rt_handle.ptr += SIZE_T(backbuffer_index) * SIZE_T(rt_descriptor_size);
    command_list->ClearRenderTargetView(rt_handle, bg_color, 0, nullptr);
    command_list->ClearDepthStencilView(ds_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify which buffers will be used in rendering
    command_list->OMSetRenderTargets(1, &rt_handle, true, &ds_handle);
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