#pragma once
#include <cstdint>

namespace SableUI
{
    struct Texture
    {
        Texture() = default;
        Texture(int width, int height);
        ~Texture();

        Texture(const Texture& other) = delete;
        Texture& operator=(const Texture& other) = delete;

        void Resize(int width, int height);
        void SetColour(uint32_t v);

        uint32_t* pixels = nullptr;
        int width = 0;
        int height = 0;
    };
}