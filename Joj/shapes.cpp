#include "shapes.h"

#include <d3dcompiler.h>
#include "error.h"
#include "engine.h"

void Shapes::init()
{
    // controla rotação do cubo
    theta = DirectX::XM_PIDIV4;
    phi = DirectX::XM_PIDIV4;
    radius = 10.0f;

    // pega última posição do mouse
    last_xmouse = (f32)input->get_xmouse();
    last_ymouse  = (f32)input->get_ymouse();

    // inicializa as matrizes World e View para a identidade
    World = View = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    // inicializa a matriz de projeção
    XMStoreFloat4x4(&Proj, DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        window->get_aspect_ratio(),
        1.0f, 100.0f));

    JojEngine::Engine::renderer->reset_commands();

    // Build geometry and initialize pipeline
    build_descriptor_heaps();
    build_constant_buffers();
    build_geometry();
    build_root_signature();
    build_pipeline_state();

    JojEngine::Engine::renderer->submit_commands();
}

void Shapes::update()
{
    // Exit with ESCAPE key
    if (input->is_key_press(VK_ESCAPE))
        window->close();

    f32 xmouse = (f32)input->get_xmouse();
    f32 ymouse = (f32)input->get_ymouse();

    if (input->is_key_down(VK_LBUTTON))
    {
        // cada pixel corresponde a 1/4 de grau
        f32 dx = DirectX::XMConvertToRadians(0.4f * (xmouse - last_xmouse));
        f32 dy = DirectX::XMConvertToRadians(0.4f * (ymouse - last_ymouse));

        // atualiza ângulos com base no deslocamento do mouse 
        // para orbitar a câmera ao redor da caixa
        theta += dx;
        phi += dy;

        // restringe o ângulo de phi ]0-180[ graus
        phi = phi < 0.1f ? 0.1f : (phi > (DirectX::XM_PI - 0.1f) ? DirectX::XM_PI - 0.1f : phi);
    }
    else if (input->is_key_down(VK_RBUTTON))
    {
        // cada pixel corresponde a 0.05 unidades
        f32 dx = 0.05f * (xmouse - last_xmouse);
        f32 dy = 0.05f * (ymouse - last_ymouse);

        // atualiza o raio da câmera com base no deslocamento do mouse 
        radius += dx - dy;

        // restringe o raio (3 a 15 unidades)
        radius = radius < 3.0f ? 3.0f : (radius > 15.0f ? 15.0f : radius);
    }

    last_xmouse = xmouse;
    last_ymouse = ymouse;

    // converte coordenadas esféricas para cartesianas
    f32 x = radius * sinf(phi) * cosf(theta);
    f32 z = radius * sinf(phi) * sinf(theta);
    f32 y = radius * cosf(phi);

    // constrói a matriz da câmera (view matrix)
    DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&View, view);

    // constrói matriz combinada (world x view x proj)
    DirectX::XMMATRIX world = XMLoadFloat4x4(&World);
    DirectX::XMMATRIX proj = XMLoadFloat4x4(&Proj);
    DirectX::XMMATRIX WorldViewProj = world * view * proj;

    // Update constant buffer with combined matrix (Word-View-Projection Matrix)
    JojRenderer::ObjectConstant obj_constant;
    XMStoreFloat4x4(&obj_constant.world_view_proj, DirectX::XMMatrixTranspose(WorldViewProj));
    memcpy(constant_buffer_data, &obj_constant, sizeof(JojRenderer::ObjectConstant));
}

void Shapes::draw()
{
    JojEngine::Engine::renderer->clear(pipeline_state);

    // Submit pipeline configuration commands
    ID3D12DescriptorHeap* descriptor_heaps[] = { constant_buffer_heap };
    JojEngine::Engine::dx12_graphics->get_command_list()->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);

    JojEngine::Engine::dx12_graphics->get_command_list()->SetGraphicsRootSignature(root_signature);

    vertex_buffer_view.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
    vertex_buffer_view.StrideInBytes = vertex_byte_stride;
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetVertexBuffers(0, 1, &vertex_buffer_view);

    index_buffer_view.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
    index_buffer_view.Format = index_format;
    index_buffer_view.SizeInBytes = index_buffer_size;
    JojEngine::Engine::dx12_graphics->get_command_list()->IASetIndexBuffer(&index_buffer_view);

    JojEngine::Engine::dx12_graphics->get_command_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    JojEngine::Engine::dx12_graphics->get_command_list()->SetGraphicsRootDescriptorTable(0, constant_buffer_heap->GetGPUDescriptorHandleForHeapStart());

    // Submit Drawing Commands
    JojEngine::Engine::dx12_graphics->get_command_list()->DrawIndexedInstanced(geo.get_index_count() , 1, 0, 0, 0);

    JojEngine::Engine::renderer->present();
}

void Shapes::shutdown()
{
    constant_buffer_upload->Unmap(0, nullptr);
    constant_buffer_upload->Release();
    constant_buffer_heap->Release();

    root_signature->Release();
    pipeline_state->Release();
}

void Shapes::build_descriptor_heaps()
{
    // Constant buffer descriptor
    D3D12_DESCRIPTOR_HEAP_DESC constant_buffer_heap_desc = {};
    constant_buffer_heap_desc.NumDescriptors = 1;
    constant_buffer_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    constant_buffer_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    // Create descriptor for constant buffer
    JojEngine::Engine::renderer->get_device()->CreateDescriptorHeap(&constant_buffer_heap_desc, IID_PPV_ARGS(&constant_buffer_heap));
}

void Shapes::build_constant_buffers()
{
    // Upload buffer heap properties
    D3D12_HEAP_PROPERTIES upload_heap_properties = {};
    upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    upload_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    upload_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    upload_heap_properties.CreationNodeMask = 1;
    upload_heap_properties.VisibleNodeMask = 1;

    /* The size of the "Constant Buffers" must be multiple
      of the minimum allocation size of the hardware (256 bytes) */
    u32 constant_buffer_size = (sizeof(JojRenderer::ObjectConstant) + 255) & ~255;

    // Upload buffer descriptor
    D3D12_RESOURCE_DESC upload_buffer_desc = {};
    upload_buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    upload_buffer_desc.Alignment = 0;
    upload_buffer_desc.Width = constant_buffer_size;
    upload_buffer_desc.Height = 1;
    upload_buffer_desc.DepthOrArraySize = 1;
    upload_buffer_desc.MipLevels = 1;
    upload_buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
    upload_buffer_desc.SampleDesc.Count = 1;
    upload_buffer_desc.SampleDesc.Quality = 0;
    upload_buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    upload_buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create an upload buffer for the constant buffer
    JojEngine::Engine::renderer->get_device()->CreateCommittedResource(
        &upload_heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &upload_buffer_desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constant_buffer_upload));

    // Upload buffer address in GPU
    D3D12_GPU_VIRTUAL_ADDRESS upload_address = constant_buffer_upload->GetGPUVirtualAddress();

    // Describe constant buffer view
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
    cbv_desc.BufferLocation = upload_address;
    cbv_desc.SizeInBytes = constant_buffer_size;

    // Create a view for constant buffer
    JojEngine::Engine::renderer->get_device()->CreateConstantBufferView(
        &cbv_desc,
        constant_buffer_heap->GetCPUDescriptorHandleForHeapStart());

    // Map memory from the upload buffer to a CPU-accessible address
    constant_buffer_upload->Map(0, nullptr, reinterpret_cast<void**>(&constant_buffer_data));
}

void Shapes::build_geometry()
{
    // --------------------------------
    // Vertex Buffer
    // --------------------------------

    // Geometries
    geo = JojRenderer::Cube(2.0f, 2.0f, 2.0f);
    //geo = JojRenderer::Cylinder(1.0f, 0.5f, 3.0f, 10, 10);
    //geo = JojRenderer::Sphere(1.0f, 20, 20);
    //geo = JojRenderer::GeoSphere(1.0f, 3);
    //geo = JojRenderer::Grid(20.0f, 20.0f, 20, 20);
    //geo = JojRenderer::Quad(3.0f, 1.0f);

    // -----------------------------------------------------------
    // >> Allocate and Copy Vertex and Index Buffers to the GPU <<
    // -----------------------------------------------------------

    // Byte size of vertices and indexes
    const u32 vb_size = geo.get_vertex_count() * sizeof(JojRenderer::Vertex);
    const u32 ib_size = geo.get_index_count() * sizeof(u32);

    // Setup geometry attributes (Mesh)
    vertex_byte_stride = sizeof(JojRenderer::Vertex);
    vertex_buffer_size = vb_size;
    index_format = DXGI_FORMAT_R32_UINT;
    index_buffer_size = ib_size;

    // Allocate resources to the Vertex Buffer
    JojEngine::Engine::renderer->allocate_resource_in_cpu(vb_size, &vertex_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, vb_size, &vertex_buffer_upload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, vb_size, &vertex_buffer_gpu);

    // Allocate resources to the Index Buffer
    JojEngine::Engine::renderer->allocate_resource_in_cpu(ib_size, &index_buffer_cpu);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::UPLOAD, ib_size, &index_buffer_upload);
    JojEngine::Engine::renderer->allocate_resource_in_gpu(JojRenderer::AllocationType::GPU, ib_size, &index_buffer_gpu);

    // Save a copy of the vertices and indexes in the 'mesh'
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(geo.get_vertex_data(), vb_size, vertex_buffer_cpu);
    JojEngine::Engine::renderer->copy_verts_to_cpu_blob(geo.get_index_data(), ib_size, index_buffer_cpu);

    // Copy vertices and indexes to the GPU using the Upload buffer
    JojEngine::Engine::renderer->copy_verts_to_gpu(geo.get_vertex_data(), vb_size, vertex_buffer_upload, vertex_buffer_gpu);
    JojEngine::Engine::renderer->copy_verts_to_gpu(geo.get_index_data(), ib_size, index_buffer_upload, index_buffer_gpu);
}

void Shapes::build_root_signature()
{
    // TODO: comment specifications on cbv_table
    // Create a single table of CBV descriptors
    D3D12_DESCRIPTOR_RANGE cbv_table = {};
    cbv_table.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    cbv_table.NumDescriptors = 1;
    cbv_table.BaseShaderRegister = 0;
    cbv_table.RegisterSpace = 0;
    cbv_table.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // TODO: comment specifications on root_parameters
    // Root parameter can be a table, root descriptor, or root constant
    D3D12_ROOT_PARAMETER root_parameters[1];
    root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    root_parameters[0].DescriptorTable.NumDescriptorRanges = 1;
    root_parameters[0].DescriptorTable.pDescriptorRanges = &cbv_table;

    // TODO: comment specifications on root_sig_desc
    // Describe empty root signature
    D3D12_ROOT_SIGNATURE_DESC root_sig_desc = {};
    root_sig_desc.NumParameters = 1;
    root_sig_desc.pParameters = root_parameters;
    root_sig_desc.NumStaticSamplers = 0;
    root_sig_desc.pStaticSamplers = nullptr;
    root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // Serialize Root Signature
    ID3DBlob* serialized_root_sig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &root_sig_desc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serialized_root_sig,
        &error));

    if (error != nullptr)
        OutputDebugString((char*)error->GetBufferPointer());

    /* Create a root signature with a single slot that points to
     a range of descriptors consisting of a single constant buffer */
    ThrowIfFailed(JojEngine::Engine::renderer->get_device()->CreateRootSignature(
        0,
        serialized_root_sig->GetBufferPointer(),
        serialized_root_sig->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)));
}

void Shapes::build_pipeline_state()
{
    // --------------------------------
    // Input Assembler
    // --------------------------------

    // Setup vertex description
    D3D12_INPUT_ELEMENT_DESC input_layout[2] =
    {
        {
            "POSITION",                                     // Semantic name
            0,                                              // Semantic index
            DXGI_FORMAT_R32G32B32_FLOAT,                    // Format of this vertex element (3D 32-bit f32 vector)
            0,                                              // Input slot index this element will come from
            0,                                              // Aligned byte offset
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     // Input slot class (simple technique of instancing)
            0                                               // Instance data step rate (0 = no advanced technique of instancing)
        },
        {
            "COLOR",                                        // Semantic name
            0,                                              // Semantic index
            DXGI_FORMAT_R32G32B32A32_FLOAT,                 // Format of this vertex element (3D 32-bit f32 vector)
            0,                                              // Input slot index this element will come from
            12,                                             // Aligned byte offset (4 bytes * XMFLOAT3 = 12)
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     // Input slot class (simple technique of instancing)
            0                                               // Instance data step rate (0 = no advanced technique of instancing)
        }
    };

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertex_shader;
    ID3DBlob* pixel_shader;

    D3DReadFileToBlob(L"../vertex.cso", &vertex_shader);
    D3DReadFileToBlob(L"../pixel.cso", &pixel_shader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

    // TODO: comment specifications on rasterizer
    // Describe rasterizer
    D3D12_RASTERIZER_DESC rasterizer = {};
    //rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
    rasterizer.CullMode = D3D12_CULL_MODE_BACK;
    //rasterizer.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ---------------------
    // --- Color Blender ---
    // ---------------------

    // TODO: comment specifications on blender
    // Describe blender
    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    for (u32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = default_render_target_blend_desc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    // TODO: comment specifications on depth_stencil
    // Describe Depth/Stencil
    D3D12_DEPTH_STENCIL_DESC depth_stencil = {};
    depth_stencil.DepthEnable = TRUE;
    depth_stencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth_stencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depth_stencil.StencilEnable = FALSE;
    depth_stencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depth_stencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC default_stencil_op =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depth_stencil.FrontFace = default_stencil_op;
    depth_stencil.BackFace = default_stencil_op;

    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    // TODO: comment specifications on pso
    // Describe Depth/Stencil
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = root_signature;
    pso.VS = { reinterpret_cast<BYTE*>(vertex_shader->GetBufferPointer()), vertex_shader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixel_shader->GetBufferPointer()), pixel_shader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depth_stencil;
    pso.InputLayout = { input_layout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = JojEngine::Engine::dx12_graphics->get_antialiasing();
    pso.SampleDesc.Quality = JojEngine::Engine::dx12_graphics->get_quality();
    JojEngine::Engine::dx12_graphics->get_device()->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline_state));

    vertex_shader->Release();
    pixel_shader->Release();
}