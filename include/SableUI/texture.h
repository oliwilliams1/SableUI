#pragma once
#include <cstdint>
#include <GL/glew.h>

namespace SableUI
{
    struct Texture
    {
        Texture() = default;
        Texture(int width, int height);
        ~Texture();

        Texture(const Texture& other) = delete;
        Texture& operator=(const Texture& other) = delete;

        void initGPUTexture();
        void Resize(int width, int height);
        void Resize(float width, float height);

        void Bind() const;

        int width = 0;
        int height = 0;

        GLuint texID = 0;

    private:
        void Update() const;
    };
}