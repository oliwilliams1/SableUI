#include <memory>
#include <algorithm>
#include "SableUI/drawable.h"
#include "SableUI/renderer.h"
#include "SableUI/texture.h"

void SableUI::DrawableRect::Update(SableUI::Rect& rect, SableUI::Colour colour, float pBSize)
{
    this->r = rect;
    this->c = colour;
}

void SableUI::DrawableRect::Draw(SableUI::Texture* texture)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (r.x / static_cast<float>(texture->width));
    float y = (r.y / static_cast<float>(texture->height));
    float w = (r.w / static_cast<float>(texture->width));
    float h = (r.h / static_cast<float>(texture->height));

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

    glColor3ub(c.r, c.g, c.b);

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
    this->r = rect;
    this->c = colour;
    this->type = type;
    this->b = SableUI::f2i(pBSize);
    this->offsets = segments;
}

void SableUI::DrawableSplitter::Draw(SableUI::Texture* texture)
{
    glColor3f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);

    int x = std::clamp(SableUI::f2i(r.x), 0, texture->width - 1);
    int y = std::clamp(SableUI::f2i(r.y), 0, texture->height - 1);
    int width = std::clamp(SableUI::f2i(r.w), 0, texture->width - x);
    int height = std::clamp(SableUI::f2i(r.h), 0, texture->height - y);

    glBegin(GL_QUADS);

    switch (type)
    {
    case SableUI::NodeType::HSPLITTER:
        for (int offset : offsets)
        {
            float drawX = x + static_cast<float>(offset) - b;
            if (drawX >= 0 && drawX < texture->width)
            {
                /* freaky never again */
                glVertex2f((drawX / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (y / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((drawX + b * 2) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (y / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((drawX + b * 2) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - ((y + height) / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f((drawX / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - ((y + height) / static_cast<float>(texture->height)) * 2.0f);
            }
        }
        break;

    case SableUI::NodeType::VSPLITTER:
        for (int offset : offsets)
        {
            float drawY = y + static_cast<float>(offset) - b;
            if (drawY >= 0 && drawY < texture->height)
            {
                glVertex2f((x / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((x + width) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY / static_cast<float>(texture->height)) * 2.0f);
                glVertex2f(((x + width) / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY + b * 2) / static_cast<float>(texture->height) * 2.0f);
                glVertex2f((x / static_cast<float>(texture->width)) * 2.0f - 1.0f,
                    1.0f - (drawY + b * 2) / static_cast<float>(texture->height) * 2.0f);
            }
        }
        break;
    }

    glEnd();
}

/* image */
void SableUI::DrawableImage::Draw(SableUI::Texture* texture)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (r.x / static_cast<float>(texture->width));
    float y = (r.y / static_cast<float>(texture->height));
    float w = (r.w / static_cast<float>(texture->width));
    float h = (r.h / static_cast<float>(texture->height));

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
    t.Bind();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); 
    glVertex2f(x, y);
    
    glTexCoord2f(1.0f, 1.0f); 
    glVertex2f(x + w, y);
    
    glTexCoord2f(1.0f, 0.0f); 
    glVertex2f(x + w, y + h);
    
    glTexCoord2f(0.0f, 0.0f); 
    glVertex2f(x, y + h);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}