#pragma once

#include "defines.h"

#include <DirectXMath.h>
#include <vector>
#include <DirectXColors.h>
#include "dx12/renderer_dx12.h"

namespace JojRenderer
{
	enum class GeometryType { UNKNOWN, CUBE, CYLINDER, SPHERE, GEOSPHERE, GRID, QUAD};

	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};

	// -------------------------------------------------------------------------------
	// Geometry
	// -------------------------------------------------------------------------------

	class Geometry
	{
	public:
		Geometry();
		virtual ~Geometry();

		std::vector<Vertex> vertices;					// Geometry vertices
		std::vector<u32> indices;						// Geometry indices

		virtual f32 x() const { return position.x;  }	// Return x position of geometry
		virtual f32 y() const { return position.y;  }	// Return y position of geometry
		virtual f32 z() const { return position.z;  }	// Return z position of geometry

		// Return geometry position
		virtual DirectX::XMFLOAT3 get_position() const { return position; }
		
		// Return geometry type
		virtual GeometryType get_type() const { return type; }

		// Move geometry by delta (dx,dy,dz)
		virtual void translate(f32 dx, f32 dy, f32 dz)
		{ position.x += dx; position.y += dy; position.z += dz; }

		// Move geometry to position (px, py, pz)
		virtual void move_to(f32 px, f32 py, f32 pz)
		{ position.x = px; position.y = py; position.z = pz; }

		// Return geometry vertices
		const Vertex* get_vertex_data() const
		{ return vertices.data(); }

		// Return geometry indices
		const u32* get_index_data() const
		{ return indices.data(); }

		// Return number of geometry vertices
		u32 get_vertex_count() const
		{ return u32(vertices.size()); }

		// Return number of geometry indices
		u32 get_index_count() const
		{ return u32(indices.size()); }

	protected:
		DirectX::XMFLOAT3 position;			// Geometry position
		GeometryType type;					// Geometry type

		void subdivide();					// Subdivide triangles

	};


	// -------------------------------------------------------------------------------
	// Cube
	// -------------------------------------------------------------------------------

	class Cube : public Geometry
	{
	public:
		Cube();
		Cube(f32 width, f32 height, f32 depth);
	};

	// -------------------------------------------------------------------------------
	// Cylinder
	// -------------------------------------------------------------------------------

	class Cylinder : public Geometry
	{
	public:
		Cylinder();
		Cylinder(f32 bottom_radius, f32 top_radius, f32 height, u32 slide_count, u32 layer_count);
	};

	// -------------------------------------------------------------------------------
	// Sphere
	// -------------------------------------------------------------------------------

	class Sphere : public Geometry
	{
	public:
		Sphere();
		Sphere(f32 radius, u32 slide_count, u32 layer_count);
	};

	// -------------------------------------------------------------------------------
	// GeoSphere
	// -------------------------------------------------------------------------------

	class GeoSphere : public Geometry
	{
	public:
		GeoSphere();
		GeoSphere(f32 radius, u32 subdivisions);
	};

	// -------------------------------------------------------------------------------
	// Grid
	// -------------------------------------------------------------------------------

	class Grid : public Geometry
	{
	public:
		Grid();
		// m = lines, n = columns
		Grid(f32 width, f32 height, u32 m, u32 n);
	};

	// -------------------------------------------------------------------------------
	// Quad
	// -------------------------------------------------------------------------------

	class Quad : public Geometry
	{
	public:
		Quad();
		Quad(f32 width, f32 height);
	};
}