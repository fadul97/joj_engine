#pragma once

#include "game.h"

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

	u32 vbo = 0;
	u32 vao = 0;
	u32 shader_program = 0;
};