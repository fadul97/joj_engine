#pragma once

#include "game.h"
#include "geometry.h"
#include "fmath.h"
#include "opengl/shader.h"

class GLApp : public JojEngine::Game
{
public:
	void init();
	void update();
	void draw();
	void shutdown();

private:
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
		"color = vec4(1.0f, 0.2f, 0.6f, 1.0f);\n"
		"}\n\0";

	const char* geo_shader = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"uniform mat4 transform;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = transform * vec4(pos, 1.0);\n"
		"}\0";

	u32 vbo = 0;
	u32 vao = 0;
	u32 ebo = 0;
	i32 shader_program = 0;
	JojRenderer::Shader shader = JojRenderer::Shader{};

	JojRenderer::Cube geo = {};
	//JojRenderer::GeoSphere geo = {};

	// Camera settings
	Mat4 World = {};
	Mat4 View = {};
	Mat4 Proj = {};

	f32 theta = 0;
	f32 phi = 0;
	f32 radius = 0;

	f32 last_xmouse = 0;
	f32 last_ymouse = 0;
};