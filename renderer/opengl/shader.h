#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include <string>

#define JOJ_GL_DEFINE_EXTERN
#include "opengl/joj_gl.h"
#include "fmath.h"
#include <DirectXMath.h>

namespace JojRenderer
{
    enum class ShaderType { VERTEX, FRAGMENT, PROGRAM };

    class Shader
    {
    public:
        Shader();
        Shader(const char* vertex_path, const char* fragment_path);
        ~Shader();

        i32 get_id() const;
        
        void compile_shaders(const char* vertex_shader, const char* fragment_shader);

        void use();
        void set_bool(const std::string& name, bool value) const;
        void set_int(const std::string& name, i32 value) const;
        void set_float(const std::string& name, f32 value) const;
        void set_vec3(const std::string& name, f32 x, f32 y, f32 z) const;
        void set_vec4(const std::string& name, f32 x, f32 y, f32 z, f32 w) const;
        void set_mat4(const std::string& name, const Mat4 mat) const;
        void set_dxmat4(const std::string& name, const DirectX::XMMATRIX mat) const;

    private:
        u32 id;

        void check_compile_errors(u32 shader, ShaderType type);
    };

    inline i32 Shader::get_id() const
    { return id; }

    inline void Shader::use()
    { glUseProgram(id); }

    inline void Shader::set_bool(const std::string& name, bool value) const
    { glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<i32>(value)); }
    
    inline void Shader::set_int(const std::string& name, i32 value) const
    { glUniform1i(glGetUniformLocation(id, name.c_str()), value); }
    
    inline void Shader::set_float(const std::string& name, f32 value) const
    { glUniform1i(glGetUniformLocation(id, name.c_str()), value); }

    inline void Shader::set_vec3(const std::string& name, f32 x, f32 y, f32 z) const
    { glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); }

    inline void Shader::set_vec4(const std::string& name, f32 x, f32 y, f32 z, f32 w) const
    { glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); }

    inline void Shader::set_mat4(const std::string& name, const Mat4 mat) const
    { glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, mat.data); }

    inline void Shader::set_dxmat4(const std::string& name, const DirectX::XMMATRIX mat) const
    {
        // Convert DirectX::XMMATRIX to a compatible format
        float mat_array[16];
        memcpy(mat_array, &mat, sizeof(float) * 16);
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, mat_array);
    }
}

#endif // PLATFORM_WINDOWS