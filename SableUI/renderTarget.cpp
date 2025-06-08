#include "SableUI/renderTarget.h"
#include "SableUI/console.h"
#include "SableUI/utils.h"

SableUI::RenderTarget::RenderTarget(int width, int height)
    : width(width), height(height)
{
    initGPUTexture();
}

SableUI::RenderTarget::~RenderTarget()
{
    glDeleteTextures(1, &m_texID);
}

void SableUI::RenderTarget::initGPUTexture()
{
    if (target == TargetType::WINDOW)
    {
        SableUI_Warn("Cannot create GPU texture for texture target of window");
        return;
    };

    if (m_texID == 0)
    {
        glGenTextures(1, &m_texID);
    }

    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SableUI::RenderTarget::Resize(int w, int h)
{
    bool u = false;

    if (w != width || h != height)
	{
		u = true;
	}
    width = w;
    height = h;

    if (u && target != TargetType::WINDOW) Update();
}

void SableUI::RenderTarget::Resize(float w, float h)
{
    Resize(SableUI::f2i(w), SableUI::f2i(h));
}

void SableUI::RenderTarget::Update() const
{
    if (target == TargetType::WINDOW)
    {
        SableUI_Warn("Cannot update GPU texture for texture target of window");
        return;
    };

    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void SableUI::RenderTarget::Bind() const
{
    if (target == TargetType::WINDOW)
    {
        SableUI_Warn("Cannot bind GPU texture for texture target of window");
        return;
    };

    glBindTexture(GL_TEXTURE_2D, m_texID);
}
