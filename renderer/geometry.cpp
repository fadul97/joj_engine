#include "geometry.h"

#include <DirectXMath.h>
using namespace DirectX;

// ==============================================================================
// Geometry
// ==============================================================================

JojRenderer::Geometry::Geometry() :
    position(0, 0, 0),
    type(GeometryType::UNKNOWN)
{
}

// ------------------------------------------------------------------------------

JojRenderer::Geometry::~Geometry()
{
}

// ------------------------------------------------------------------------------

void JojRenderer::Geometry::subdivide()
{
    // Save a copy of the original geometry
    std::vector <JojRenderer::Vertex> vertices_copy = vertices;
    std::vector <u32> indices_copy = indices;

    vertices.resize(0);
    indices.resize(0);

    //       v1
    //       *
    //      / \
    //     /   \
    //  m0*-----*m1
    //   / \   / \
    //  /   \ /   \
    // *-----*-----*
    // v0    m2     v2

    u32 num_tris = (u32)indices_copy.size() / 3;
    for (u32 i = 0; i < num_tris; ++i)
    {
        JojRenderer::Vertex v0 = vertices_copy[indices_copy[size_t(i) * 3 + 0]];
        JojRenderer::Vertex v1 = vertices_copy[indices_copy[size_t(i) * 3 + 1]];
        JojRenderer::Vertex v2 = vertices_copy[indices_copy[size_t(i) * 3 + 2]];

        // Find center points of each edge
        JojRenderer::Vertex m0, m1, m2;
        XMStoreFloat3(&m0.pos, 0.5f * (XMLoadFloat3(&v0.pos) + XMLoadFloat3(&v1.pos)));
        XMStoreFloat3(&m1.pos, 0.5f * (XMLoadFloat3(&v1.pos) + XMLoadFloat3(&v2.pos)));
        XMStoreFloat3(&m2.pos, 0.5f * (XMLoadFloat3(&v0.pos) + XMLoadFloat3(&v2.pos)));

        // Add new geometry
        vertices.push_back(v0); // 0
        vertices.push_back(v1); // 1
        vertices.push_back(v2); // 2
        vertices.push_back(m0); // 3
        vertices.push_back(m1); // 4
        vertices.push_back(m2); // 5

        indices.push_back(i * 6 + 0);
        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 5);

        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 4);
        indices.push_back(i * 6 + 5);

        indices.push_back(i * 6 + 5);
        indices.push_back(i * 6 + 4);
        indices.push_back(i * 6 + 2);

        indices.push_back(i * 6 + 3);
        indices.push_back(i * 6 + 1);
        indices.push_back(i * 6 + 4);
    }
}

// ==============================================================================
// Cube
// ==============================================================================

JojRenderer::Cube::Cube()
{
    type = GeometryType::CUBE;
}

JojRenderer::Cube::Cube(f32 width, f32 height, f32 depth)
{
    type = GeometryType::CUBE;

    f32 w = 0.5f * width;
    f32 h = 0.5f * height;
    f32 d = 0.5f * depth;

    // Create geometry vertices
    Vertex cube_vertices[8] =
    {
        { DirectX::XMFLOAT3(-w, -h, -d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-w, +h, -d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, +h, -d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, -h, -d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-w, -h, +d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-w, +h, +d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, +h, +d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, -h, +d), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }
    };

    // Add vertices to mesh
    for (const Vertex& v : cube_vertices)
        vertices.push_back(v);

    // Indicate how vertices are interconnected
    u32 cube_indices[36] =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 7, 5,
        7, 6, 5,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    // Add indices to mesh
    for (u16 i : cube_indices)
        indices.push_back(i);
}

// ==============================================================================
// Cylinder
// ==============================================================================

JojRenderer::Cylinder::Cylinder()
{
    type = GeometryType::CYLINDER;
}

JojRenderer::Cylinder::Cylinder(f32 bottom, f32 top, f32 height, u32 slice_count, u32 layer_count)
{
    type = GeometryType::CYLINDER;

    // Layer height
    f32 layer_height = height / layer_count;

    // Increment in the radius of each layer
    f32 radius_step = (top - bottom) / layer_count;

    // Number of cylinder rings
    u32 ring_count = layer_count + 1;

    // Calculate vertices of each ring
    for (u32 i = 0; i < ring_count; ++i)
    {
        f32 y = -0.5f * height + i * layer_height;
        f32 r = bottom + i * radius_step;
        f32 theta = 2.0f * DirectX::XM_PI / slice_count;

        for (u32 j = 0; j <= slice_count; ++j)
        {
            f32 c = cosf(j * theta);
            f32 s = sinf(j * theta);

            Vertex vertex;
            vertex.pos = DirectX::XMFLOAT3(r * c, y, r * s);
            vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);
            vertices.push_back(vertex);
        }
    }

    // Number of vertices in each cylinder ring
    u32 ring_vertex_count = slice_count + 1;

    // Calculate indexes for each layer
    for (u32 i = 0; i < layer_count; ++i)
    {
        for (u32 j = 0; j < slice_count; ++j)
        {
            indices.push_back(i * ring_vertex_count + j);
            indices.push_back((i + 1) * ring_vertex_count + j);
            indices.push_back((i + 1) * ring_vertex_count + j + 1);
            indices.push_back(i * ring_vertex_count + j);
            indices.push_back((i + 1) * ring_vertex_count + j + 1);
            indices.push_back(i * ring_vertex_count + j + 1);
        }
    }

    // Constructs vertices of cylinder covers
    for (u32 k = 0; k < 2; ++k)
    {
        u32 base_index = u32(vertices.size());

        f32 y = (k - 0.5f) * height;
        f32 theta = 2.0f * DirectX::XM_PI / slice_count;
        f32 r = (k ? top : bottom);
        Vertex vertex;

        for (u32 i = 0; i <= slice_count; i++)
        {
            f32 x = r * cosf(i * theta);
            f32 z = r * sinf(i * theta);

            vertex.pos = DirectX::XMFLOAT3(x, y, z);
            vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);
            vertices.push_back(vertex);
        }

        // Central vertex of the lid
        vertex.pos = DirectX::XMFLOAT3(0.0f, y, 0.0f);
        vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);
        vertices.push_back(vertex);

        u32 center_index = u32(vertices.size() - 1);

        // Indices for the lid
        for (u32 i = 0; i < slice_count; ++i)
        {
            indices.push_back(center_index);
            indices.push_back(base_index + i + k);
            indices.push_back(base_index + i + 1 - k);
        }
    }
}

// ==============================================================================
// Sphere
// ==============================================================================

JojRenderer::Sphere::Sphere()
{
    type = GeometryType::SPHERE;
}

JojRenderer::Sphere::Sphere(f32 radius, u32 slice_count, u32 layer_count)
{
    type = GeometryType::SPHERE;

    // Calculate the vertex by starting at the top pole and working its way down through the layers

    Vertex top_vertex;
    top_vertex.pos = DirectX::XMFLOAT3(0.0f, radius, 0.0f);
    top_vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);

    Vertex bottom_vertex;
    bottom_vertex.pos = DirectX::XMFLOAT3(0.0f, -radius, 0.0f);
    bottom_vertex.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);

    vertices.push_back(top_vertex);

    f32 phiStep = DirectX::XM_PI / layer_count;
    f32 thetaStep = 2.0f * DirectX::XM_PI / slice_count;

    // Calculate the vertices for each ring (does not count the poles as rings)
    for (u32 i = 1; i <= layer_count - 1; ++i)
    {
        f32 phi = i * phiStep;

        // Ring vertices
        for (u32 j = 0; j <= slice_count; ++j)
        {
            f32 theta = j * thetaStep;

            Vertex v;

            // Spherical coordinates for Cartesian coordinates
            v.pos.x = radius * sinf(phi) * cosf(theta);
            v.pos.y = radius * cosf(phi);
            v.pos.z = radius * sinf(phi) * sinf(theta);

            v.color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);

            vertices.push_back(v);
        }
    }

    vertices.push_back(bottom_vertex);

    // Calculate the indexes of the top layer
    // This layer connects the top pole to the first ring
    for (u32 i = 1; i <= slice_count; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // Calculate the indexes for the inner layers (not connected to the poles)
    u32 base_index = 1;
    u32 ring_vertex_count = slice_count + 1;
    for (u32 i = 0; i < layer_count - 2; ++i)
    {
        for (u32 j = 0; j < slice_count; ++j)
        {
            indices.push_back(base_index + i * ring_vertex_count + j);
            indices.push_back(base_index + i * ring_vertex_count + j + 1);
            indices.push_back(base_index + (i + 1) * ring_vertex_count + j);

            indices.push_back(base_index + (i + 1) * ring_vertex_count + j);
            indices.push_back(base_index + i * ring_vertex_count + j + 1);
            indices.push_back(base_index + (i + 1) * ring_vertex_count + j + 1);
        }
    }

    // Calculate the indexes of the bottom layer
    // This layer connects the bottom pole to the last ring

    // Bottom pole is added last
    u32 south_pole_index = (u32)vertices.size() - 1;

    // It's positioned at the indices of the first vertex of the last ring
    base_index = south_pole_index - ring_vertex_count;

    for (u32 i = 0; i < slice_count; ++i)
    {
        indices.push_back(south_pole_index);
        indices.push_back(base_index + i);
        indices.push_back(base_index + i + 1);
    }
}

// ==============================================================================
// GeoSphere
// ==============================================================================

JojRenderer::GeoSphere::GeoSphere()
{
    type = GeometryType::GEOSPHERE;
}

JojRenderer::GeoSphere::GeoSphere(f32 radius, u32 subdivisions)
{
    type = GeometryType::GEOSPHERE;

    // Max number of subdivisions is 6
    subdivisions = (subdivisions > 6U ? 6U : subdivisions);

    // Approximate a sphere by the subdivision of an icosahedron
    const f32 X = 0.525731f;
    const f32 Z = 0.850651f;

    // Vertices of the icosahedron
    DirectX::XMFLOAT3 pos[12] =
    {
        DirectX::XMFLOAT3(-X, 0.0f, Z),  DirectX::XMFLOAT3(X, 0.0f, Z),
        DirectX::XMFLOAT3(-X, 0.0f, -Z), DirectX::XMFLOAT3(X, 0.0f, -Z),
        DirectX::XMFLOAT3(0.0f, Z, X),   DirectX::XMFLOAT3(0.0f, Z, -X),
        DirectX::XMFLOAT3(0.0f, -Z, X),  DirectX::XMFLOAT3(0.0f, -Z, -X),
        DirectX::XMFLOAT3(Z, X, 0.0f),   DirectX::XMFLOAT3(-Z, X, 0.0f),
        DirectX::XMFLOAT3(Z, -X, 0.0f),  DirectX::XMFLOAT3(-Z, -X, 0.0f)
    };

    // Indices of the icosahedron
    u32 k[60] =
    {
        1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
        1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
        3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
        10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
    };

    // Snap and initialize vertex vectors and indices
    vertices.resize(12);
    indices.assign(&k[0], &k[60]);

    for (u32 i = 0; i < 12; ++i)
        vertices[i].pos = pos[i];

    // Subdivide each triangle of the icosahedron a certain number of times
    for (u32 i = 0; i < subdivisions; ++i)
        subdivide();

    // Project the vertices onto a sphere and adjust the scale
    for (u32 i = 0; i < vertices.size(); ++i)
    {
        // Normalize vetor (point)
        DirectX::XMVECTOR n = DirectX::XMVector3Normalize(XMLoadFloat3(&vertices[i].pos));

        // Project on sphere
        DirectX::XMVECTOR p = radius * n;

        XMStoreFloat3(&vertices[i].pos, p);
        vertices[i].color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);
    }
}

// ==============================================================================
// Grid
// ==============================================================================

JojRenderer::Grid::Grid()
{
    type = GeometryType::GRID;
}

JojRenderer::Grid::Grid(f32 width, f32 depth, u32 m, u32 n)
{
    type = GeometryType::GRID;

    u32 vertex_count = m * n;
    u32 triangle_count = 2 * (m - 1) * (n - 1);

    // Create vertices
    f32 half_width = 0.5f * width;
    f32 half_depth = 0.5f * depth;

    f32 dx = width / (n - 1);
    f32 dz = depth / (m - 1);

    // Adjust vertex vector size
    vertices.resize(vertex_count);

    for (u32 i = 0; i < m; ++i)
    {
        f32 z = half_depth - i * dz;

        for (u32 j = 0; j < n; ++j)
        {
            f32 x = -half_width + j * dx;

            // Define grid vertices
            vertices[size_t(i) * n + j].pos = DirectX::XMFLOAT3(x, 0.0f, z);
            vertices[size_t(i) * n + j].color = DirectX::XMFLOAT4(DirectX::Colors::Yellow);
        }
    }

    // Adjust vector size of indexes
    indices.resize(size_t(triangle_count) * 3);

    size_t k = 0;

    for (u32 i = 0; i < m - 1; ++i)
    {
        for (u32 j = 0; j < n - 1; ++j)
        {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1) * n + j;
            indices[k + 3] = (i + 1) * n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = (i + 1) * n + j + 1;

            k += 6; // Next quad
        }
    }
}

// ==============================================================================
// Quad
// ==============================================================================

JojRenderer::Quad::Quad()
{
    type = GeometryType::QUAD;
}

// ------------------------------------------------------------------------------

JojRenderer::Quad::Quad(f32 width, f32 height)
{
    type = GeometryType::QUAD;

    f32 w = 0.5f * width;
    f32 h = 0.5f * height;

    // Create vertex buffer
    Vertex quad_vertices[4] =
    {
        { DirectX::XMFLOAT3(-w, -h, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(-w, +h, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, +h, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) },
        { DirectX::XMFLOAT3(+w, -h, 0.0f), DirectX::XMFLOAT4(DirectX::Colors::Yellow) }
    };

    // Add vertices to mesh
    for (const Vertex& v : quad_vertices)
        vertices.push_back(v);

    // Create index buffer
    u32 quad_indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    // insere índices na malha
    for (u32 i : quad_indices)
        indices.push_back(i);
}