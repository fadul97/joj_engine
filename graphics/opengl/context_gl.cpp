#include "context_gl.h"

#include "joj_gl.h"
#include "logger.h"

typedef BOOL(WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

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

JojGraphics::GLContext::GLContext()
{
    rc = nullptr;
    color_bits = 32;
    depth_bits = 24;

    const u32 pxf_attrib_list[] =
    {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, color_bits,
      WGL_DEPTH_BITS_ARB, depth_bits,
      0
    };

    memcpy(pixel_format_attrib_list, pxf_attrib_list, sizeof(pxf_attrib_list));

    gl_version_major = 4;
    gl_version_minor = 6;

    const int ctx_attribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, gl_version_major,
      WGL_CONTEXT_MINOR_VERSION_ARB, gl_version_minor,
      WGL_CONTEXT_FLAGS_ARB,
#ifdef _DEBUG
      WGL_CONTEXT_DEBUG_BIT_ARB |
#endif // _DEBUG
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };

    memcpy(context_attribs, ctx_attribs, sizeof(ctx_attribs));
}

JojGraphics::GLContext::~GLContext()
{

}

b8 JojGraphics::GLContext::init(std::unique_ptr<JojPlatform::Window>& window)
{
    auto dummy_window = new JojPlatform::Window();
    dummy_window->create();

    // TODO: Add comments about pfd
    // Describe pixel format
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        (BYTE)depth_bits,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        (BYTE)color_bits,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    u32 pixel_format = ChoosePixelFormat(dummy_window->get_device_context(), &pfd);
    if (!pixel_format)
    {
        FFATAL(ERR_CONTEXT, "Failed to ChoosePixelFormat for dummy_window.");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    if (!SetPixelFormat(dummy_window->get_device_context(), pixel_format, &pfd))
    {
        FFATAL(ERR_CONTEXT, "Failed to SetPixelFormat for dummy_window.");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    HGLRC new_rc = wglCreateContext(dummy_window->get_device_context());
    if (!new_rc)
    {
        FFATAL(ERR_CONTEXT, "Failed to wglCreateContext for dummy_window.");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    if (!wglMakeCurrent(dummy_window->get_device_context(), new_rc))
    {
        OutputDebugString("Failed to wglMakeCurrent for dummy_window.");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        OutputDebugString("Failed to wglGetProcAddress of wglChoosePixelFormatARB.\n");
        return false;
    }

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
    {
        OutputDebugString("Failed to wglGetProcAddress of wglCreateContextAttribsARB.\n");
        return false;
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(new_rc);
    DestroyWindow(dummy_window->get_id());

    int new_pixel_format;
    int num_pixel_formats = 0;
    PIXELFORMATDESCRIPTOR new_pfd;

    const int* pxf_attrib_list = (const int*)pixel_format_attrib_list;
    const int* context_attrib_list = (const int*)context_attribs;

    wglChoosePixelFormatARB(window->get_device_context(), pxf_attrib_list, nullptr, 1, &new_pixel_format, (UINT*)&num_pixel_formats);
    if (num_pixel_formats <= 0)
    {
        FFATAL(ERR_CONTEXT, "Failed to wglChoosePixelFormatARB.");
        return false;
    }

    if (!SetPixelFormat(window->get_device_context(), new_pixel_format, &new_pfd))
    {
        FFATAL(ERR_CONTEXT, "Failed to SetPixelFormat.");
        return false;
    }

    new_rc = wglCreateContextAttribsARB(window->get_device_context(), 0, context_attrib_list);
    if (!new_rc)
    {
        FFATAL(ERR_CONTEXT, "Failed to wglCreateContextAttribsARB.");
        return false;
    }

    rc = new_rc;

    if (!wglMakeCurrent(window->get_device_context(), rc))
    {
        FFATAL(ERR_CONTEXT, "Failed to wglMakeCurrent of window.");
        return false;
    }

    load_opengl_functions();

    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SwapBuffers(window->get_device_context());

#if _DEBUG
    log_hardware_info();
#endif // _DEBUG

    return true;
}
void JojGraphics::GLContext::log_hardware_info()
{
    FDEBUG("OpenGL Version: %s.", glGetString(GL_VERSION));
    FDEBUG("OpenGL Renderer: %s.", glGetString(GL_RENDERER));
    FDEBUG("OpenGL GLSL Version: %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));
}