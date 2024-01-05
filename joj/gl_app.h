#pragma once

#include "game.h"
#include "fmath.h"
#include "opengl/shader.h"
#include "opengl/quad.h"
#include "geometry.h"
#include "DirectXMath.h"
#include "opengl/camera.h"

class GLApp : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

	void process_camera_input();
	void mouse_callback(double xposIn, double yposIn);

private:
	void build_buffers();

	// Define the vertex shader source code
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 position;\n"
		"void main()\n"
		"{\n"
		"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
		"}\0";

	// Define the fragment shader source code
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"color = vec4(1.0f, 0.4f, 0.6f, 1.0f);\n"
		"}\n\0";

	const char* geo_vertex = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"layout (location = 1) in vec4 objectColor;\n"
		"out vec4 vertColor;\n"
		"uniform mat4 transform;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = transform * vec4(pos, 1.0);\n"
		"	vertColor = objectColor;\n"
		"}\0";

	// Define the fragment shader source code
	const char* geo_frag = "#version 330 core\n"
		"out vec4 fragColor;\n"
		"in vec4 vertColor;\n"
		"uniform vec3 objectColor;\n"
		"uniform vec3 lightColor;\n"
		"void main()\n"
		"{\n"
		"	vec4 light = vec4(lightColor, 1.0); "
		"	fragColor = light * vertColor;\n"
		"}\n\0";

	// Define the fragment shader source code
	const char* light_frag = "#version 330 core\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"	color = vec4(1.0);\n"
		"}\n\0";

	const char* r_vert = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(pos, 1.0);\n"
		"}\0";

	const char* r_frag = "#version 330 core\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"	vec2 uv = gl_FragCoord.xy; // position of the fragment\n"
		"	vec2 pos = 2.0 * uv - 1.0; // convert uv to [-1, 1] range\n"
		"	float curve = 0.5; // amount of curve\n"
		"	vec2 d = abs(pos)-vec2(0.3, 0.3);\n"
		"	float dist = length(max(d,0.0)) - curve; // distance from center\n"
		"	if (dist > 0.3)\n"
		"	{\n"
		"		discard; // discard fragments outside the radius\n"
		"	}\n"
		"	else\n"
		"	{\n"
		"		color = vec4(0.0f, 0.0f, 1.0f, 1.0f); // color of the quad\n"
		"	}\n"
		"}\n\0";

	const char* vshader_path = "../shaders/vert.glsl";
	const char* vfrag_path = "../shaders/frag.glsl";

	u32 vbo = 0;
	u32 vao = 0;
	u32 ebo = 0;
	i32 shader_program = 0;
	JojRenderer::Shader shader;

	JojRenderer::Cube geo;


	// Light settings
	//JojRenderer::Cube light_cube = JojRenderer::Cube{ 1.0f, 1.0f, 1.0f, DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f} };
	//JojRenderer::Shader light_shader;
	//u32 light_vao;



	// Camera settings
	DirectX::XMFLOAT4X4 World = {};
	DirectX::XMFLOAT4X4 View = {};
	DirectX::XMFLOAT4X4 Proj = {};

	// Camera object
	JojRenderer::Camera camera = JojRenderer::Camera{ DirectX::XMFLOAT3{ 0.0f, 0.0f, 3.0f } };
	f32 lastX;
	f32 lastY;
	bool firstMouse = true;
	i32 centerX;
	i32 centerY;
	i32 cmouseX;
	i32 cmouseY;
};



inline DirectX::XMFLOAT3 vec3_to_dxf3(Vec3 v)
{
	return DirectX::XMFLOAT3{ v.x, v.y, v.z };
}

inline Vec3 dxf3_to_vec3(DirectX::XMFLOAT3 v)
{
	return Vec3{ v.x, v.y, v.z };
}

inline DirectX::XMFLOAT4 vec4_to_dx4(Vec4 v)
{
	return DirectX::XMFLOAT4{ v.x, v.y, v.z, v.w };
}

inline Vec4 dxf4_to_vec4(DirectX::XMFLOAT4 v)
{
	return Vec4{ v.x, v.y, v.z, v.w };
}

inline DirectX::XMMATRIX mat4_to_dxmat(Mat4 m)
{
	return DirectX::XMMATRIX{ m.data };
}

inline Mat4 dxmat_to_mat4(DirectX::XMMATRIX m)
{
	DirectX::XMFLOAT4X4 aux;
	XMStoreFloat4x4(&aux, m);
	return Mat4{ 
		aux._11, aux._12, aux._13, aux._14,
		aux._21, aux._22, aux._23, aux._24,
		aux._31, aux._32, aux._33, aux._34,
		aux._41, aux._42, aux._43, aux._44
	};
}

inline Mat4 dxmat_to_mat4trans(DirectX::XMMATRIX m)
{
	DirectX::XMFLOAT4X4 aux;
	XMStoreFloat4x4(&aux, m);
	return Mat4{
		aux._11, aux._21, aux._31, aux._41,
		aux._12, aux._22, aux._32, aux._42,
		aux._13, aux._23, aux._33, aux._43,
		aux._14, aux._24, aux._34, aux._44
	};
}