#pragma once
#include <cstdint>
#include <GL/glew.h>

namespace SableUI
{
    enum class TargetType
    {
        WINDOW,
        TEXTURE
    };

    struct RenderTarget
    {
        RenderTarget() = default;
        RenderTarget(int width, int height);
        ~RenderTarget();

        RenderTarget(const RenderTarget& other) = delete;
        RenderTarget& operator=(const RenderTarget& other) = delete;

        void SetTarget(TargetType target) { this->target = target; }
        void initGPUTexture();
        void Resize(int width, int height);
        void Resize(float width, float height);

        void Bind() const;

        int width = 0;
        int height = 0;

		TargetType target = TargetType::WINDOW;

    private:
        GLuint m_texID = 0;
        void Update() const;
    };
}