#pragma once

#include <GL/glew.h>
#include <string>

struct Shader {
public:
    Shader() = default;
    ~Shader();
    void LoadBasicShaders(const char* vsSource, const char* fsSource);
    void Use() const;

    GLuint shaderProgram;

private:
    std::string vsSource, fsSource;
};