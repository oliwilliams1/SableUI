#pragma once

#include <GL/glew.h>
#include <string>

namespace SableUI
{
    void DestroyShaders();

    struct Shader {
    public:
        Shader() = default;
        ~Shader() = default;
        void LoadBasicShaders(const char* vsSource, const char* fsSource);
        void Use() const;

        GLuint m_shaderProgram = 0;

    private:
        std::string vsSource, fsSource;
    };
}