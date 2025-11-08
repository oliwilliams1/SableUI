#pragma once

#include <string>
#include <memory>
#include <chrono>

namespace SableUI
{
    void StepCachedTexturesCleaner();

    struct CachedGpuTexture;
    struct Texture
    {
        Texture();
        Texture(int width, int height, uint32_t texID);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        static int GetNumInstances();

        void LoadTexture(const std::string& path);
        void LoadTextureOptimised(const std::string& path, int width = -1, int height = -1);

        void SetDefaultTexture(uint32_t texID);
        void Bind();

        int m_width = -1;
        int m_height = -1;

    private:
        void GenerateDefaultTexture();

        uint32_t m_defaultTexID = 0;
        std::shared_ptr<CachedGpuTexture> m_cachedGpu;
    };
}
