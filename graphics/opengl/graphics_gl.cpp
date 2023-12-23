#include "graphics_gl.h"

#include <iostream>

JojGraphics::GLGraphics::GLGraphics()
{
    bg_color[0] = 0.0f;
    bg_color[1] = 0.0f;
    bg_color[2] = 0.0f;
    bg_color[3] = 1.0f;
}

JojGraphics::GLGraphics::~GLGraphics()
{

}

b8 JojGraphics::GLGraphics::init(JojPlatform::Window* window)
{
    this->window = window;

    int colorBits = 32;
    int depthBits = 24;

    int glVersionMajor = 4;
    int glVersionMinor = 6;

    // -----------------------------------------------------
    // Create Temporary Window
    // -----------------------------------------------------

    auto dummy_window = new JojPlatform::Window();
    dummy_window->set_mode(JojPlatform::WindowMode::WINDOWED);
    dummy_window->set_size(800, 600);
    dummy_window->set_color(60, 60, 60);
    dummy_window->set_title("Joj Engine");
    dummy_window->create();

    // TODO: Add comments about pfd
    // Describe pixel format
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        (BYTE)depthBits,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        (BYTE)colorBits,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixel_format = ChoosePixelFormat(dummy_window->get_device_context(), &pfd);
    if (!pixel_format)
    {
        OutputDebugString("Failed to ChoosePixelFormat for dummy_window.\n");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    if (!SetPixelFormat(dummy_window->get_device_context(), pixel_format, &pfd))
    {
        OutputDebugString("Failed to SetPixelFormat for dummy_window.\n");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    HGLRC rc = wglCreateContext(dummy_window->get_device_context());
    if (!rc)
    {
        OutputDebugString("Failed to wglCreateContext for dummy_window.\n");
        DestroyWindow(dummy_window->get_id());
        return false;
    }

    dummy_window->set_rendering_context(rc);
    if (!wglMakeCurrent(dummy_window->get_device_context(), dummy_window->get_rendering_context()))
    {
        OutputDebugString("Failed to wglMakeCurrent for dummy_window.\n");
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

    if (!wglMakeCurrent(0, 0))
    {
        OutputDebugString("Failed to wglMakeCurrent of dummy_window.\n");
        return false;
    }

    if (!wglDeleteContext(dummy_window->get_rendering_context()))
    {
        OutputDebugString("Failed to wglDeleteContext of dummy_window.\n");
        return false;
    }

    if (!DeleteDC(dummy_window->get_device_context()))
    {
        OutputDebugString("Failed to DeleteDC of dummy_window.\n");
        return false;
    }

    if (!DestroyWindow(dummy_window->get_id()))
    {
        OutputDebugString("Failed to DestroyWindow of dummy_window.\n");
        return false;
    }

    const int pixelFormatAttribList[] =
    {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, colorBits,
      WGL_DEPTH_BITS_ARB, depthBits,
      0
    };

    int num_pixel_formats = 0;
    wglChoosePixelFormatARB(window->get_device_context(), pixelFormatAttribList, nullptr, 1, &pixel_format, (UINT*)&num_pixel_formats);
    if (num_pixel_formats <= 0)
    {
        OutputDebugString("Failed to wglChoosePixelFormatARB.\n");
        return false;
    }

    if (!SetPixelFormat(window->get_device_context(), pixel_format, &pfd))
    {
        OutputDebugString("Failed to SetPixelFormat.\n");
        return false;
    }

    const int context_attribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, glVersionMajor,
      WGL_CONTEXT_MINOR_VERSION_ARB, glVersionMinor,
      WGL_CONTEXT_FLAGS_ARB,
#if DEBUG
      WGL_CONTEXT_DEBUG_BIT_ARB |
#endif // DEBUG
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };

    rc = wglCreateContextAttribsARB(window->get_device_context(), 0, context_attribs);
    if (!rc)
    {
        OutputDebugString("Failed to wglCreateContextAttribsARB.\n");
        return false;
    }

    window->set_rendering_context(rc);

    if (!wglMakeCurrent(window->get_device_context(), window->get_rendering_context()))
    {
        OutputDebugString("Failed to wglMakeCurrent.\n");
        return false;
    }

    // Get OpenGL function Pointers here
    HMODULE opengl32Dll = GetModuleHandleA("OpenGL32.dll");
    if (!opengl32Dll)
    {
        OutputDebugString("Failed to GetModuleHandleA of OpenGL32.dll.\n");
        return false;
    }

    glClearColor = (PFNGLCLEARCOLORPROC)GetProcAddress(opengl32Dll, "glClearColor");
    if (!glClearColor)
    {
        OutputDebugString("Failed to GetProcAddress of glClearColor.\n");
        return false;
    }

    glClear = (PFNGLCLEARPROC)GetProcAddress(opengl32Dll, "glClear");
    if (!glClear)
    {
        OutputDebugString("Failed to GetProcAddress of glClear.\n");
        return false;
    }

    b8 result = load_extension_list();
    if (!result)
    {
        OutputDebugString("Failed to load_extension_list.\n");
        return false;
    }

    const GLubyte* version = glGetString(GL_VERSION);
    if (version)
    {
        const char* versionString = reinterpret_cast<const char*>(version);
        OutputDebugString("OpenGL Version: ");
        OutputDebugString(versionString);
    }
    else
    {
        OutputDebugString("Failed to get OpenGL version.\n");
    }

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(window->get_device_context());

    // Define the vertex data for the triangle
    f32 vertices[] = {
        -0.5f, -0.5f, 0.0f, // Left
         0.5f, -0.5f, 0.0f, // Right
         0.0f,  0.5f, 0.0f  // Top
    };


    // Create a vbo and a vao
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    // Bind the vbo and the vao
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Fill the vbo with the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind the vbo and the vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create a vertex shader object
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Attach the vertex shader source code to the shader object
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // Compile the vertex shader
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::VERTEX::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Create a fragment shader object
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Attach the fragment shader source code to the shader object
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // Compile the fragment shader
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Create a shader program object
    shader_program = glCreateProgram();

    // Attach the vertex and fragment shaders to the shader program
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragmentShader);

    // Link the shader program
    glLinkProgram(shader_program);

    // Check for shader program linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        OutputDebugString("ERROR::SHADER::PROGRAM::COMPILATION_FAILED:\n");
        OutputDebugString(infoLog);
    }

    // Delete the vertex and fragment shaders as they are no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Use the shader program
    glUseProgram(shader_program);

    return true;
}

void JojGraphics::GLGraphics::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    glUseProgram(shader_program);
    glBindVertexArray(vao);         // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SwapBuffers(window->get_device_context());
}

void JojGraphics::GLGraphics::swap_buffers()
{

}

b8 JojGraphics::GLGraphics::load_extension_list()
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