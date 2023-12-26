#include "renderer_dx12.h"

#include "logger.h"
#include <d3dcompiler.h>

JojRenderer::DX12Renderer::DX12Renderer()
{
    context = std::make_unique<JojGraphics::DX12Context>();

    // Graphics Infrastructure
    device = nullptr;
    backbuffer_index = 0;   // 0 is the first

    // Configuration
    backbuffer_count = 2;	// Double buffering
    antialiasing = 1;		// No antialising
    quality = 0;			// Default quality
    vsync = false;			// No vertical sync
    
    // Backbuffer background color
    bg_color[0] = 0.0f;		// Red
    bg_color[1] = 0.0f;		// Green
    bg_color[2] = 0.0f;		// Blue
    bg_color[3] = 0.0f;		// Alpha (0 = transparent, 1 = solid)

    // ---------------------------------------------------
    // Pipeline members
    // ---------------------------------------------------
    command_queue = nullptr;
    command_list = nullptr;
    command_list_alloc = nullptr;

    // CPU/GPU Synchronization
    fence = nullptr;
    current_fence = 0;
   
    swapchain = nullptr;
    render_target_heap = nullptr;
    rt_descriptor_size = 0;
    render_targets = new ID3D12Resource * [backbuffer_count] {nullptr};
    
    depth_stencil = nullptr;
    depth_stencil_heap = nullptr;
    
    ZeroMemory(&viewport, sizeof(viewport));
    ZeroMemory(&scissor_rect, sizeof(scissor_rect));

}

JojRenderer::DX12Renderer::~DX12Renderer()
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
}

b8 JojRenderer::DX12Renderer::init(std::unique_ptr<JojPlatform::Window>& window)
{
    // Initialize DX12 context
    context->init(window);

    // Get pointer to D3D11 Device
    device = context->get_device();

    // ------------------------------------------------------------------------------------------------------
    //                                          PIPELINE SETUP
    // ------------------------------------------------------------------------------------------------------

    // ---------------------------------------------------
    // Create queue, list and command allocator
    // ---------------------------------------------------

    // Create GPU command queue
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if FAILED(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)))
    {
        FFATAL(ERR_RENDERER, "Failed to create command queue.");
        return false;
    }

    // Create command allocator
    if FAILED(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&command_list_alloc)))
    {
        FFATAL(ERR_RENDERER, "Failed to create command allocator.");
        return false;
    }

    // Create command list
    if FAILED(device->CreateCommandList(
        0,										// Using only one GPU
        D3D12_COMMAND_LIST_TYPE_DIRECT,			// Does not inherit state on the GPU
        command_list_alloc,						// Command allocator
        nullptr,								// Pipeline initial state
        IID_PPV_ARGS(&command_list)))			// Command list object
    {
        FFATAL(ERR_RENDERER, "Failed to create command list.");
        return false;
    }

    // ---------------------------------------------------
    // Create fence to synchronize CPU/GPU
    // ---------------------------------------------------

    if FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))
    {
        FFATAL(ERR_RENDERER, "Failed to create D3D12 fence.");
        return false;
    }

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
    if FAILED(context->get_factory()->CreateSwapChainForHwnd(
        command_queue,                          // GPU Command Queue
        window->get_id(),                       // Window ID
        &swapchain_desc,                        // Swap chain descriptor
        nullptr,                                // Fullscreen Swap Chain
        nullptr,                                // Restrict output screen
        &swapchain))                            // Swap chain object
    {
        FFATAL(ERR_RENDERER, "Failed to create swap chain.");
        return false;
    }

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
    if FAILED(device->CreateDescriptorHeap(&render_target_heap_desc, IID_PPV_ARGS(&render_target_heap)))
    {
        FFATAL(ERR_RENDERER, "Failed to create descriptor heap for render target.");
        return false;
    }

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
    // Depth/Stencil View (and associated heaps)
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
    if FAILED(device->CreateCommittedResource(
        &ds_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &depth_stencil_desc,
        D3D12_RESOURCE_STATE_COMMON,
        &optmized_clear,
        IID_PPV_ARGS(&depth_stencil)))
    {
        FFATAL(ERR_RENDERER, "Failed to create committed resource.");
        return false;
    }

    // TODO: comment specifications on depthstencil_heap_desc
    // Specify Depth/Stencil heap descriptor
    D3D12_DESCRIPTOR_HEAP_DESC depth_stencil_heap_desc = {};
    depth_stencil_heap_desc.NumDescriptors = 1;
    depth_stencil_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    depth_stencil_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    // Create heap for Depth/Stencil descriptor
    if FAILED(device->CreateDescriptorHeap(&depth_stencil_heap_desc, IID_PPV_ARGS(&depth_stencil_heap)))
    {
        FFATAL(ERR_RENDERER, "Failed to create descriptor heap for depth stencil.");
        return false;
    }

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

    return true;
}

void JojRenderer::DX12Renderer::render()
{
    // TODO:
}

void JojRenderer::DX12Renderer::clear()
{
}

void JojRenderer::DX12Renderer::swap_buffers()
{
    // Indicates the buffer will be used for presentation
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = render_targets[backbuffer_index];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &barrier);

    // Submit command list to the GPU
    submit_commands();

    // Show frame and swap front/back buffers
    swapchain->Present(vsync, 0);
    backbuffer_index = (backbuffer_index + 1) % backbuffer_count;
}

void JojRenderer::DX12Renderer::shutdown()
{
}

void JojRenderer::DX12Renderer::custom_clear(ID3D12PipelineState* pso)
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
    if FAILED(command_queue->Signal(fence, current_fence))
    {
        FERROR(ERR_RENDERER, "Failed to process command queue signal.");
        return false;
    }

    // Wait for GPU to complete all previous commands
    if (fence->GetCompletedValue() < current_fence)
    {
        HANDLE event_handle = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);

        if (event_handle)
        {
            // Trigger event when GPU reaches current fence
            if FAILED(fence->SetEventOnCompletion(current_fence, event_handle))
            {
                FERROR(ERR_RENDERER, "Failed to set event on completion for D3D12 fence.");
                return false;
            }

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

void JojRenderer::DX12Renderer::reset_commands()
{
    // Restart list of commands to prepare for the startup commands
    command_list->Reset(command_list_alloc, nullptr);
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
    if FAILED(device->CreateCommittedResource(
        &buffer_prop,
        D3D12_HEAP_FLAG_NONE,
        &buffer_desc,
        init_state,
        nullptr,
        IID_PPV_ARGS(resource)))
    {
        FERROR(ERR_RENDERER, "Failed to create buffer for commited resource.");
        return;
    }
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