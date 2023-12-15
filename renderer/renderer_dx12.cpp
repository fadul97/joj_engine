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
	//if (factory)
		//factory->Release();
}

b8 JojRenderer::DX12Renderer::init(JojPlatform::Window* window, JojGraphics::DX12Graphics* graphics)
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

void JojRenderer::DX12Renderer::reset_commands()
{
    // Restart list of commands to prepare for the startup commands
    command_list->Reset(command_list_alloc, nullptr);
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

void JojRenderer::DX12Renderer::allocate_resource_in_cpu(u32 size_in_bytes, ID3DBlob** resource)
{
    D3DCreateBlob(size_in_bytes, resource);
}

void JojRenderer::DX12Renderer::allocate_resource_in_gpu(AllocationType alloc_type, u32 size_in_bytes, ID3D12Resource** resource)
{
    // TODO: comment specifications on buffer_prop
    // Buffer heap properties
    D3D12_HEAP_PROPERTIES buffer_prop = {};
    buffer_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    buffer_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    buffer_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    buffer_prop.CreationNodeMask = 1;
    buffer_prop.VisibleNodeMask = 1;

    if (alloc_type == AllocationType::UPLOAD)
        buffer_prop.Type = D3D12_HEAP_TYPE_UPLOAD;

    // TODO: comment specifications on buffer_desc
    // Buffer description
    D3D12_RESOURCE_DESC buffer_desc = {};
    buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer_desc.Alignment = 0;
    buffer_desc.Width = size_in_bytes;
    buffer_desc.Height = 1;
    buffer_desc.DepthOrArraySize = 1;
    buffer_desc.MipLevels = 1;
    buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
    buffer_desc.SampleDesc.Count = 1;
    buffer_desc.SampleDesc.Quality = 0;
    buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_STATES init_state = D3D12_RESOURCE_STATE_COMMON;

    if (alloc_type == AllocationType::UPLOAD)
        init_state = D3D12_RESOURCE_STATE_GENERIC_READ;

    // Create a buffer for resource
    ThrowIfFailed(device->CreateCommittedResource(
        &buffer_prop,
        D3D12_HEAP_FLAG_NONE,
        &buffer_desc,
        init_state,
        nullptr,
        IID_PPV_ARGS(resource)));
}

void JojRenderer::DX12Renderer::copy_verts_to_cpu_blob(const void* vertices, u32 size_in_bytes, ID3DBlob* buffer_cpu)
{
    CopyMemory(buffer_cpu->GetBufferPointer(), vertices, size_in_bytes);
}

/* @brief Copy vertices to the default buffer (GPU)
 * To copy data to the GPU:
 *  - First, copy the data to the intermediate upload heap
 *  - Then, ID3D12CommandList::CopyBufferRegion copies itself from upload to GPU
 */
void JojRenderer::DX12Renderer::copy_verts_to_gpu(const void* vertices, u32 size_in_bytes, ID3D12Resource* buffer_upload, ID3D12Resource* buffer_gpu)
{
    // Describe the data that will be copied
    D3D12_SUBRESOURCE_DATA vertex_sub_resource_data = {};
    vertex_sub_resource_data.pData = vertices;
    vertex_sub_resource_data.RowPitch = size_in_bytes;
    vertex_sub_resource_data.SlicePitch = size_in_bytes;

    // Describe the layout of the video memory (GPU)
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts;
    u32 num_rows;
    u64 row_size_in_bytes;
    u64 required_size = 0;

    // Get GPU buffer description
    D3D12_RESOURCE_DESC buffer_gpu_desc = buffer_gpu->GetDesc();

    // Get video memory layout
    device->GetCopyableFootprints(
        &buffer_gpu_desc,
        0, 1, 0, &layouts, &num_rows,
        &row_size_in_bytes, &required_size);

    // Lock Upload Buffer memory for exclusive access
    BYTE* pData;
    buffer_upload->Map(0, nullptr, (void**)&pData);

    // Describe the destiny of a copy operation
    D3D12_MEMCPY_DEST dest_data =
    {
        pData + layouts.Offset,
        layouts.Footprint.RowPitch,
        layouts.Footprint.RowPitch * u64(num_rows)
    };

    // Copy vertices into the upload buffer
    for (u32 z = 0; z < layouts.Footprint.Depth; ++z)
    {
        // Destiny address
        BYTE* dest_slice = (BYTE*)(dest_data.pData) + dest_data.SlicePitch * z;

        // Source address
        const BYTE* srcSlice = (const BYTE*)(vertex_sub_resource_data.pData) + vertex_sub_resource_data.SlicePitch * z;

        // Copy line by line
        for (u32 y = 0; y < num_rows; ++y)
            memcpy(dest_slice + dest_data.RowPitch * y,
                srcSlice + vertex_sub_resource_data.RowPitch * y,
                (size_t)row_size_in_bytes);
    }

    // Release Memory Lock from Upload Buffer
    buffer_upload->Unmap(0, nullptr);

    // Change GPU memory state (from read to write)
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = buffer_gpu;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &barrier);

    // Copy vertex buffer from upload buffer to GPU
    command_list->CopyBufferRegion(
        buffer_gpu,
        0,
        buffer_upload,
        layouts.Offset,
        layouts.Footprint.Width);

    // Change GPU memory state (from write to read)
    barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = buffer_gpu;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &barrier);
}