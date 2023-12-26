#include "context_gl.h"

#include "logger.h"

PFNGLATTACHSHADERPROC JojGraphics::GLContext::glAttachShader = nullptr;
PFNGLBINDBUFFERPROC JojGraphics::GLContext::glBindBuffer = nullptr;
PFNGLBINDVERTEXARRAYPROC JojGraphics::GLContext::glBindVertexArray = nullptr;
PFNGLBUFFERDATAPROC JojGraphics::GLContext::glBufferData = nullptr;
PFNGLCOMPILESHADERPROC JojGraphics::GLContext::glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC JojGraphics::GLContext::glCreateProgram = nullptr;
PFNGLCREATESHADERPROC JojGraphics::GLContext::glCreateShader = nullptr;
PFNGLDELETEBUFFERSPROC JojGraphics::GLContext::glDeleteBuffers = nullptr;
PFNGLDELETEPROGRAMPROC JojGraphics::GLContext::glDeleteProgram = nullptr;
PFNGLDELETESHADERPROC JojGraphics::GLContext::glDeleteShader = nullptr;
PFNGLDELETEVERTEXARRAYSPROC JojGraphics::GLContext::glDeleteVertexArrays = nullptr;
PFNGLDETACHSHADERPROC JojGraphics::GLContext::glDetachShader = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC JojGraphics::GLContext::glEnableVertexAttribArray = nullptr;
PFNGLGENBUFFERSPROC JojGraphics::GLContext::glGenBuffers = nullptr;
PFNGLGENVERTEXARRAYSPROC JojGraphics::GLContext::glGenVertexArrays = nullptr;
PFNGLGETATTRIBLOCATIONPROC JojGraphics::GLContext::glGetAttribLocation = nullptr;
PFNGLGETPROGRAMINFOLOGPROC JojGraphics::GLContext::glGetProgramInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC JojGraphics::GLContext::glGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC JojGraphics::GLContext::glGetShaderInfoLog = nullptr;
PFNGLGETSHADERIVPROC JojGraphics::GLContext::glGetShaderiv = nullptr;
PFNGLLINKPROGRAMPROC JojGraphics::GLContext::glLinkProgram = nullptr;
PFNGLSHADERSOURCEPROC JojGraphics::GLContext::glShaderSource = nullptr;
PFNGLUSEPROGRAMPROC JojGraphics::GLContext::glUseProgram = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC JojGraphics::GLContext::glVertexAttribPointer = nullptr;
PFNGLBINDATTRIBLOCATIONPROC JojGraphics::GLContext::glBindAttribLocation = nullptr;
PFNGLGETUNIFORMLOCATIONPROC JojGraphics::GLContext::glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC JojGraphics::GLContext::glUniformMatrix4fv = nullptr;
PFNGLACTIVETEXTUREPROC JojGraphics::GLContext::glActiveTexture = nullptr;
PFNGLUNIFORM1IPROC JojGraphics::GLContext::glUniform1i = nullptr;
PFNGLGENERATEMIPMAPPROC JojGraphics::GLContext::glGenerateMipmap = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC JojGraphics::GLContext::glDisableVertexAttribArray = nullptr;
PFNGLUNIFORM3FVPROC JojGraphics::GLContext::glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC JojGraphics::GLContext::glUniform4fv = nullptr;
PFNGLCLEARCOLORPROC JojGraphics::GLContext::glClearColor = nullptr;
PFNGLCLEARPROC JojGraphics::GLContext::glClear = nullptr;
PFNGLDRAWARRAYSPROC JojGraphics::GLContext::glDrawArrays = nullptr;

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

    if (!load_extension_list())
    {
        FFATAL(ERR_CONTEXT, "Failed to load OpenGL extensions.");
        return false;
    }

    // Get OpenGL function Pointers here
    HMODULE opengl32Dll = GetModuleHandleA("OpenGL32.dll");
    if (!opengl32Dll)
    {
        OutputDebugString("Failed to GetModuleHandleA of OpenGL32.dll.\n");
        FFATAL(ERR_CONTEXT, "Failed to GetModuleHandleA of OpenGL32.dll.");
        return false;
    }

    glClearColor = (PFNGLCLEARCOLORPROC)GetProcAddress(opengl32Dll, "glClearColor");
    if (!glClearColor)
    {
        OutputDebugString("Failed to GetProcAddress of glClearColor.\n");
        FFATAL(ERR_CONTEXT, "Failed to GetProcAddress of glClearColor function.");
        return false;
    }

    glClear = (PFNGLCLEARPROC)GetProcAddress(opengl32Dll, "glClear");
    if (!glClear)
    {
        OutputDebugString("Failed to GetProcAddress of glClear.\n");
        FFATAL(ERR_CONTEXT, "Failed to GetProcAddress of glClear function.");
        return false;
    }

    glDrawArrays = (PFNGLDRAWARRAYSPROC)GetProcAddress(opengl32Dll, "glDrawArrays");
    if (!glDrawArrays)
    {
        OutputDebugString("Failed to GetProcAddress of glDrawArrays.\n");
        FFATAL(ERR_CONTEXT, "Failed to GetProcAddress of glDrawArrays function.");
        return false;
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
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
    //FDEBUG("OpenGL GLSL Version: %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

b8 JojGraphics::GLContext::load_extension_list()
{
    // Load the OpenGL extensions that this application will be using.
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        return false;
    }

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribsARB)
    {
        return false;
    }

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (!wglSwapIntervalEXT)
    {
        return false;
    }

    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    if (!glAttachShader)
    {
        return false;
    }

    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    if (!glBindBuffer)
    {
        return false;
    }

    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    if (!glBindVertexArray)
    {
        return false;
    }

    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    if (!glBufferData)
    {
        return false;
    }

    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    if (!glCompileShader)
    {
        return false;
    }

    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    if (!glCreateProgram)
    {
        return false;
    }

    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    if (!glCreateShader)
    {
        return false;
    }

    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    if (!glDeleteBuffers)
    {
        return false;
    }

    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    if (!glDeleteProgram)
    {
        return false;
    }

    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    if (!glDeleteShader)
    {
        return false;
    }

    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    if (!glDeleteVertexArrays)
    {
        return false;
    }

    glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
    if (!glDetachShader)
    {
        return false;
    }

    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    if (!glEnableVertexAttribArray)
    {
        return false;
    }

    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    if (!glGenBuffers)
    {
        return false;
    }

    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    if (!glGenVertexArrays)
    {
        return false;
    }

    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
    if (!glGetAttribLocation)
    {
        return false;
    }

    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    if (!glGetProgramInfoLog)
    {
        return false;
    }

    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    if (!glGetProgramiv)
    {
        return false;
    }

    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    if (!glGetShaderInfoLog)
    {
        return false;
    }

    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    if (!glGetShaderiv)
    {
        return false;
    }

    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    if (!glLinkProgram)
    {
        return false;
    }

    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    if (!glShaderSource)
    {
        return false;
    }

    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    if (!glUseProgram)
    {
        return false;
    }

    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    if (!glVertexAttribPointer)
    {
        return false;
    }

    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
    if (!glBindAttribLocation)
    {
        return false;
    }

    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    if (!glGetUniformLocation)
    {
        return false;
    }

    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    if (!glUniformMatrix4fv)
    {
        return false;
    }

    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    if (!glActiveTexture)
    {
        return false;
    }

    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    if (!glUniform1i)
    {
        return false;
    }

    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
    if (!glGenerateMipmap)
    {
        return false;
    }

    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    if (!glDisableVertexAttribArray)
    {
        return false;
    }

    glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
    if (!glUniform3fv)
    {
        return false;
    }

    glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
    if (!glUniform4fv)
    {
        return false;
    }

    return true;
}