#pragma once

#include "game.h"
#include "opengl/geometry.h"
#include "fmath.h"
#include "opengl/shader.h"
#include "opengl/quad.h"
#include "geometry.h"
#include "DirectXMath.h"

class GLApp : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

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

	const char* geo_shader = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"uniform mat4 transform;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = transform * vec4(pos, 1.0);\n"
		"}\0";

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

	//JojRenderer::Cube geo = {};
	//JojRenderer::Cylinder geo = {};
	//JojRenderer::Sphere geo = {};
	//JojRenderer::GeoSphere geo = {};
	//JojRenderer::Grid geo = {};
	JojRenderer::Quad geo = {};

	MyQuad q = MyQuad{ 1.0f, 1.0f };

	Mat4 perspective ={};
	Mat4 ortho ={};
};