#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "graphics_context.h"
#include <gl/gl.h>

/////////////
// DEFINES //
/////////////
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_CONTEXT_MAJOR_VERSION_ARB  0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB  0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_TEXTURE0                       0x84C0
#define GL_BGRA                           0x80E1
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

//////////////
// TYPEDEFS //
//////////////
typedef BOOL(WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef void (APIENTRY* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC) (GLenum target, ptrdiff_t size, const GLvoid* data, GLenum usage);
typedef void (APIENTRY* PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY* PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint* arrays);
typedef void (APIENTRY* PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
typedef GLint(APIENTRY* PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const char* name);
typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (APIENTRY* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRY* PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const char** string, const GLint* length);
typedef void (APIENTRY* PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void (APIENTRY* PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const char* name);
typedef GLint(APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const char* name);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRY* PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY* PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY* PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRY* PFNGLCLEARCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY* PFNGLCLEARPROC) (GLbitfield mask);
typedef void (APIENTRY* PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);

namespace JojGraphics
{
	class GLContext : public GraphicsContext
	{
	public:
		GLContext();
		~GLContext();

		b8 init(std::unique_ptr<JojPlatform::Window>& window);	// Initialize context
		void swap_buffers();

	private:
		HGLRC rc;		// Rendering context for Win32 Window
		u32 color_bits;
		u32 depth_bits;
		u32 pixel_format_attrib_list[16];
		u32 context_attribs[16];

		u32 gl_version_major;
		u32 gl_version_minor;

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

		void log_hardware_info();	// Show hardware information

		b8 load_extension_list();

	public:
		static PFNGLATTACHSHADERPROC glAttachShader;
		static PFNGLBINDBUFFERPROC glBindBuffer;
		static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
		static PFNGLBUFFERDATAPROC glBufferData;
		static PFNGLCOMPILESHADERPROC glCompileShader;
		static PFNGLCREATEPROGRAMPROC glCreateProgram;
		static PFNGLCREATESHADERPROC glCreateShader;
		static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
		static PFNGLDELETEPROGRAMPROC glDeleteProgram;
		static PFNGLDELETESHADERPROC glDeleteShader;
		static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
		static PFNGLDETACHSHADERPROC glDetachShader;
		static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
		static PFNGLGENBUFFERSPROC glGenBuffers;
		static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
		static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
		static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
		static PFNGLGETPROGRAMIVPROC glGetProgramiv;
		static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
		static PFNGLGETSHADERIVPROC glGetShaderiv;
		static PFNGLLINKPROGRAMPROC glLinkProgram;
		static PFNGLSHADERSOURCEPROC glShaderSource;
		static PFNGLUSEPROGRAMPROC glUseProgram;
		static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
		static PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
		static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
		static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
		static PFNGLACTIVETEXTUREPROC glActiveTexture;
		static PFNGLUNIFORM1IPROC glUniform1i;
		static PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
		static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
		static PFNGLUNIFORM3FVPROC glUniform3fv;
		static PFNGLUNIFORM4FVPROC glUniform4fv;
		static PFNGLCLEARCOLORPROC glClearColor;
		static PFNGLCLEARPROC glClear;
		static PFNGLDRAWARRAYSPROC glDrawArrays;
	};
}

#endif // PLATFORM_WINDOWS