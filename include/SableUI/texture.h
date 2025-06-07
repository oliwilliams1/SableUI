#pragma once
#include <cstdint>
#include <GL/glew.h>

namespace SableUI
{
    struct Texture
    {
        Texture() = default;
        Texture(int width, int height, bool gpuTexture = false);
        ~Texture();

        Texture(const Texture& other) = delete;
        Texture& operator=(const Texture& other) = delete;

        void initGPUTexture();
        void Resize(int width, int height);
        void SetColour(uint32_t v) const;

        void Update() const;

        void Bind() const;

        int width = 0;
        int height = 0;

        bool gpuTexture = false;
        GLuint texID = -1;
    };
}