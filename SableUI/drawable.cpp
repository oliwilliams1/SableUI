#include <memory>
#include <algorithm>
#include "SableUI/drawable.h"
#include "SableUI/renderer.h"

void SableUI::DrawableRect::Update(SableUI::Rect& rect, SableUI::Colour colour, float pBSize)
{
    this->m_rect = rect;
    this->m_colour = colour;
}

void SableUI::DrawableRect::Draw(SableUI::RenderTarget* texture)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (m_rect.x / static_cast<float>(texture->width));
    float y = (m_rect.y / static_cast<float>(texture->height));
    float w = (m_rect.w / static_cast<float>(texture->width));
    float h = (m_rect.h / static_cast<float>(texture->height));

    /* normalise to opengl NDC [0, 1] ->[-1, 1] */
    x = x * 2.0f - 1.0f;
    y = y * 2.0f - 1.0f;
    w *= 2.0f;
    h *= 2.0f;

    /* revent negative scale */
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    glColor3ub(m_colour.r, m_colour.g, m_colour.b);

    glBegin(GL_QUADS);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

void SableUI::DrawableSplitter::Update(SableUI::Rect& rect, SableUI::Colour colour, SableUI::NodeType type, 
                                       float pBSize, const std::vector<int>& segments)
{
    this->m_rect = rect;
    this->m_colour = colour;
    this->m_type = type;
    this->m_bSize = SableUI::f2i(pBSize);
    this->m_offsets = segments;
}

void SableUI::DrawableSplitter::Draw(SableUI::RenderTarget* texture)
{
    glColor3f(m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f);

    int x = std::clamp(SableUI::f2i(m_rect.x), 0, texture->width - 1);
    int y = std::clamp(SableUI::f2i(m_rect.y), 0, texture->height - 1);
    int width = std::clamp(SableUI::f2i(m_rect.w), 0, texture->width - x);
    int height = std::clamp(SableUI::f2i(m_rect.h), 0, texture->height - y);

    glBegin(GL_QUADS);

    switch (m_type)
    {
    case SableUI::NodeType::HSPLITTER:
        for (int offset : m_offsets)
        {
            float drawX = x + static_cast<float>(offset) - m_bSize;
            if (drawX >= 0 && drawX < texture->width)
            {
                /* freaky never again */
                glVertex2f((drawX / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (y / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((drawX + m_bSize * 2) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (y / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((drawX + m_bSize * 2) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - ((y + height) / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f((drawX / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - ((y + height) / static_cast<float>(texture->height)) * 2.0f);
            }
        }
        break;

    case SableUI::NodeType::VSPLITTER:
        for (int offset : m_offsets)
        {
            float drawY = y + static_cast<float>(offset) - m_bSize;
            if (drawY >= 0 && drawY < texture->height)
            {
                glVertex2f((x / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((x + width) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((x + width) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY + m_bSize * 2) / static_cast<float>(texture->height) * 2.0f);
                glVertex2f((x / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY + m_bSize * 2) / static_cast<float>(texture->height) * 2.0f);
            }
        }
        break;
    }

    glEnd();
}

/* image */
void SableUI::DrawableImage::Draw(SableUI::RenderTarget* texture)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (m_rect.x / static_cast<float>(texture->width));
    float y = (m_rect.y / static_cast<float>(texture->height));
    float w = (m_rect.w / static_cast<float>(texture->width));
    float h = (m_rect.h / static_cast<float>(texture->height));

    /* normalise to opengl NDC [0, 1] ->[-1, 1] */
    x = x * 2.0f - 1.0f;
    y = y * 2.0f - 1.0f;
    w *= 2.0f;
    h *= 2.0f;

    /* revent negative scale */
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); 
    glVertex2f(x, y);
    
    glTexCoord2f(1.0f, 0.0f); 
    glVertex2f(x + w, y);
    
    glTexCoord2f(1.0f, 1.0f); 
    glVertex2f(x + w, y + h);
    
    glTexCoord2f(0.0f, 1.0f); 
    glVertex2f(x, y + h);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}