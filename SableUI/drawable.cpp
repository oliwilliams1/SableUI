#include <memory>
#include <algorithm>
#include "SableUI/drawable.h"
#include "SableUI/renderer.h"
#include "SableUI/shader.h"

static GLuint g_rectVAO = 0;
static GLuint g_rectVBO = 0;
static GLuint g_rectEBO = 0;
static GLuint g_shaderProgram = 0;
static Shader g_rShader;
static GLuint g_rUColourLoc = 0;
static GLuint g_rURectLoc = 0;
static GLuint g_rUTexBoolLoc = 0;

struct Vertex {
    SableUI::vec2 uv;
};

Vertex rectVertices[] = {
    {{ 0.0f, 0.0f }},
    {{ 1.0f, 0.0f }},
    {{ 1.0f, 1.0f }},
    {{ 0.0f, 1.0f }}
};

unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

static GLuint GetUniformLocation(Shader shader, const char* uniformName)
{
    GLuint location = glGetUniformLocation(shader.shaderProgram, uniformName);
    if (location == -1)
    {
        SableUI_Error("Warning: %s uniform not found!", uniformName);
    }
    return location;
}

void SableUI::InitDrawables()
{
    g_rShader.LoadBasicShaders("shaders/rect.vert", "shaders/rect.frag");
    g_rShader.Use();
    g_rUColourLoc = GetUniformLocation(g_rShader, "uColour");
    g_rURectLoc = GetUniformLocation(g_rShader, "uRect");
    g_rUTexBoolLoc = GetUniformLocation(g_rShader, "uUseTexture");
    glUniform3f(g_rUColourLoc, 32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f);

    glUniform1i(glGetUniformLocation(g_rShader.shaderProgram, "uTexture"), 0);

    glGenVertexArrays(1, &g_rectVAO);
    glGenBuffers(1, &g_rectVBO);
    glGenBuffers(1, &g_rectEBO);

    glBindVertexArray(g_rectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g_rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_rectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);
}

void SableUI::DestroyDrawables()
{
    glDeleteVertexArrays(1, &g_rectVAO);
	glDeleteBuffers(1, &g_rectVBO);
	glDeleteBuffers(1, &g_rectEBO);
}

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

    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform3f(g_rUColourLoc, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f);
    glUniform1i(g_rUTexBoolLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
    g_rShader.Use();
    glUniform3f(g_rUColourLoc, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f);
    glUniform1i(g_rUTexBoolLoc, 0);

    int startX = std::clamp(SableUI::f2i(m_rect.x), 0, texture->width - 1);
    int startY = std::clamp(SableUI::f2i(m_rect.y), 0, texture->height - 1);
    int boundWidth = std::clamp(SableUI::f2i(m_rect.w), 0, texture->width - startX);
    int boundHeight = std::clamp(SableUI::f2i(m_rect.h), 0, texture->height - startY);

    switch (m_type)
    {
    case SableUI::NodeType::HSPLITTER:
    {
        for (int offset : m_offsets)
        {
            float drawX = startX + static_cast<float>(offset) - m_bSize;
            float drawWidth = m_bSize * 2;

            if (drawX + drawWidth >= 0 && drawX < texture->width)
            {
                float normalizedX = (drawX / static_cast<float>(texture->width));
                float normalizedY = (startY / static_cast<float>(texture->height));
                float normalizedW = (drawWidth / static_cast<float>(texture->width));
                float normalizedH = (boundHeight / static_cast<float>(texture->height));

                normalizedX = normalizedX * 2.0f - 1.0f;
                normalizedY = normalizedY * 2.0f - 1.0f;
                normalizedW *= 2.0f;
                normalizedH *= 2.0f;

                normalizedW = std::max(0.0f, normalizedW);
                normalizedH = std::max(0.0f, normalizedH);

                normalizedY *= -1.0f;
                normalizedH *= -1.0f;

                glUniform4f(g_rURectLoc, normalizedX, normalizedY, normalizedW, normalizedH);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        break;
    }

    case SableUI::NodeType::VSPLITTER:
    {
        for (int offset : m_offsets)
        {
            float drawY = startY + static_cast<float>(offset) - m_bSize;
            float drawHeight = m_bSize * 2;

            if (drawY + drawHeight >= 0 && drawY < texture->height)
            {
                float normalizedX = (startX / static_cast<float>(texture->width));
                float normalizedY = (drawY / static_cast<float>(texture->height));
                float normalizedW = (boundWidth / static_cast<float>(texture->width));
                float normalizedH = (drawHeight / static_cast<float>(texture->height));

                normalizedX = normalizedX * 2.0f - 1.0f;
                normalizedY = normalizedY * 2.0f - 1.0f;
                normalizedW *= 2.0f;
                normalizedH *= 2.0f;

                normalizedW = std::max(0.0f, normalizedW);
                normalizedH = std::max(0.0f, normalizedH);

                normalizedY *= -1.0f;
                normalizedH *= -1.0f;

                glUniform4f(g_rURectLoc, normalizedX, normalizedY, normalizedW, normalizedH);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
        break;
    }
    }
}

/* image */
void SableUI::DrawableImage::Draw(SableUI::RenderTarget* renderTarget)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (m_rect.x / static_cast<float>(renderTarget->width));
    float y = (m_rect.y / static_cast<float>(renderTarget->height));
    float w = (m_rect.w / static_cast<float>(renderTarget->width));
    float h = (m_rect.h / static_cast<float>(renderTarget->height));

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

    m_texture.Bind();
    
    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform1i(g_rUTexBoolLoc, 1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void SableUI::DrawableText::Draw(SableUI::RenderTarget* renderTarget)
{
    GLuint currentTexture = 0;

    float baseX = (m_rect.x / static_cast<float>(renderTarget->width));
    float baseY = (m_rect.y / static_cast<float>(renderTarget->height));

    baseX = baseX * 2.0f - 1.0f;
    baseY = baseY * 2.0f - 1.0f;

    baseY = -baseY;

    /*glEnable(GL_TEXTURE_2D);

    for (const CharDrawInfo& drInfo : m_text.m_drawInfo)
    {
        float characterX = baseX + (drInfo.relPos.x / static_cast<float>(renderTarget->width)) * 2.0f;
        float characterY = baseY + (drInfo.relPos.y / static_cast<float>(renderTarget->height)) * 2.0f;

        
        if (currentTexture != drInfo.atlasTextureID)
        {
            currentTexture = drInfo.atlasTextureID;
            glBindTexture(GL_TEXTURE_2D, currentTexture);
        }

        glBegin(GL_QUADS);
        glTexCoord2f(drInfo.uv.x, drInfo.uv.y + drInfo.uv.w);
        glVertex2f(characterX, characterY);

        glTexCoord2f(drInfo.uv.x + drInfo.uv.z, drInfo.uv.y + drInfo.uv.w);
        glVertex2f(characterX + (drInfo.size.x / static_cast<float>(renderTarget->width)) * 2.0f, characterY);

        glTexCoord2f(drInfo.uv.x + drInfo.uv.z, drInfo.uv.y);
        glVertex2f(characterX + (drInfo.size.x / static_cast<float>(renderTarget->width)) * 2.0f,
                   characterY + (drInfo.size.y / static_cast<float>(renderTarget->height)) * 2.0f);

        glTexCoord2f(drInfo.uv.x, drInfo.uv.y);
        glVertex2f(characterX, characterY + (drInfo.size.y / static_cast<float>(renderTarget->height)) * 2.0f);
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);*/
}

static unsigned int s_nextUUID = 0;

unsigned int SableUI::DrawableBase::GetUUID()
{
    return s_nextUUID++;
}

void SableUI::DrawWindowBorder(RenderTarget* target)
{
    static int borderWidth = 1;

    g_rShader.Use();
    glUniform3f(g_rUColourLoc, 51.0f / 255.0f, 51.0f / 255.0f, 51.0f / 255.0f);

    // Top
    {
        float x = -1.0f;
        float y = 1.0f;
        float w = 2.0f;
        float h = -(borderWidth * 2.0f / target->height);

        float pixelX = 0.0f;
        float pixelY = 0.0f;
        float pixelWidth = static_cast<float>(target->width);
        float pixelHeight = static_cast<float>(borderWidth);

        float ndcX = (pixelX     / static_cast<float>(target->width))   * 2.0f - 1.0f;
        float ndcY = (pixelY     / static_cast<float>(target->height))  * 2.0f - 1.0f;
        float ndcW = (pixelWidth / static_cast<float>(target->width))   * 2.0f;
        float ndcH = (pixelHeight / static_cast<float>(target->height)) * 2.0f;

        ndcW = std::max(0.0f, ndcW);
        ndcH = std::max(0.0f, ndcH);

        ndcY *= -1.0f;
        ndcH *= -1.0f;

        glUniform4f(g_rURectLoc, ndcX, ndcY, ndcW, ndcH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Bottom
    {
        float pixelX = 0.0f;
        float pixelY = static_cast<float>(target->height) - borderWidth;
        float pixelWidth = static_cast<float>(target->width);
        float pixelHeight = static_cast<float>(borderWidth);

        // Convert to NDC
        float ndcX = (pixelX / static_cast<float>(target->width)) * 2.0f - 1.0f;
        float ndcY = (pixelY / static_cast<float>(target->height)) * 2.0f - 1.0f;
        float ndcW = (pixelWidth / static_cast<float>(target->width)) * 2.0f;
        float ndcH = (pixelHeight / static_cast<float>(target->height)) * 2.0f;

        ndcW = std::max(0.0f, ndcW);
        ndcH = std::max(0.0f, ndcH);

        ndcY *= -1.0f;
        ndcH *= -1.0f;

        glUniform4f(g_rURectLoc, ndcX, ndcY, ndcW, ndcH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Left
    {
        float pixelX = 0.0f;
        float pixelY = 0.0f;
        float pixelWidth  = static_cast<float>(borderWidth);
        float pixelHeight = static_cast<float>(target->height);

        float ndcX = (pixelX     / static_cast<float>(target->width))   * 2.0f - 1.0f;
        float ndcY = (pixelY     / static_cast<float>(target->height))  * 2.0f - 1.0f;
        float ndcW = (pixelWidth / static_cast<float>(target->width))   * 2.0f;
        float ndcH = (pixelHeight / static_cast<float>(target->height)) * 2.0f;

        ndcW = std::max(0.0f, ndcW);
        ndcH = std::max(0.0f, ndcH);

        ndcY *= -1.0f;
        ndcH *= -1.0f;

        glUniform4f(g_rURectLoc, ndcX, ndcY, ndcW, ndcH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Right
    {
        float pixelX = static_cast<float>(target->width) - borderWidth;
        float pixelY = 0.0f;
        float pixelWidth = static_cast<float>(borderWidth);
        float pixelHeight = static_cast<float>(target->height);

        float ndcX = (pixelX      / static_cast<float>(target->width))  * 2.0f - 1.0f;
        float ndcY = (pixelY      / static_cast<float>(target->height)) * 2.0f - 1.0f;
        float ndcW = (pixelWidth  / static_cast<float>(target->width))  * 2.0f;
        float ndcH = (pixelHeight / static_cast<float>(target->height)) * 2.0f;

        ndcW = std::max(0.0f, ndcW);
        ndcH = std::max(0.0f, ndcH);

        ndcY *= -1.0f;
        ndcH *= -1.0f;

        glUniform4f(g_rURectLoc, ndcX, ndcY, ndcW, ndcH);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}