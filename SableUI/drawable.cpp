#include <memory>
#include <map>
#include <algorithm>
#include "SableUI/drawable.h"
#include "SableUI/shader.h"
#include "SableUI/window.h"

/* rect globals - now per-context */
static std::map<void*, SableUI::ContextResources > g_contextResources;

/* shared resources*/
static GLuint g_shaderProgram = 0;
static SableUI::Shader g_rShader;
static GLuint g_rUColourLoc = 0;
static GLuint g_rURectLoc = 0;
static GLuint g_rUTexBoolLoc = 0;

/* text globals */
static SableUI::Shader g_tShader;
static GLuint g_tTargetSizeLoc = 0;
static GLuint g_tPosLoc = 0;
static GLuint g_tAtlasLoc = 0;

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

static GLuint GetUniformLocation(SableUI::Shader shader, const char* uniformName)
{
    shader.Use();
    GLuint location = glGetUniformLocation(shader.m_shaderProgram, uniformName);
    if (location == -1)
    {
        SableUI_Error("Warning: %s uniform not found!", uniformName);
    }
    return location;
}

SableUI::ContextResources& SableUI::GetContextResources()
{
    void* ctx = SableUI::GetCurrentContext();

    auto it = g_contextResources.find(ctx);
    if (it != g_contextResources.end())
    {
        return it->second;
    }

    SableUI::ContextResources& resources = g_contextResources[ctx];

    glGenVertexArrays(1, &resources.rectVAO);
    glGenBuffers(1, &resources.rectVBO);
    glGenBuffers(1, &resources.rectEBO);

    glBindVertexArray(resources.rectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, resources.rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resources.rectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return resources;
}

void SableUI::InitDrawables()
{
    static bool shadersInitialized = false;
    if (!shadersInitialized)
    {
        g_rShader.LoadBasicShaders("shaders/rect.vert", "shaders/rect.frag");
        g_rShader.Use();
        g_rUColourLoc = GetUniformLocation(g_rShader, "uColour");
        g_rURectLoc = GetUniformLocation(g_rShader, "uRect");
        g_rUTexBoolLoc = GetUniformLocation(g_rShader, "uUseTexture");
        glUniform4f(g_rUColourLoc, 32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);
        glUniform1i(glGetUniformLocation(g_rShader.m_shaderProgram, "uTexture"), 0);

        g_tShader.LoadBasicShaders("shaders/text.vert", "shaders/text.frag");
        g_tTargetSizeLoc = GetUniformLocation(g_tShader, "uTargetSize");
        g_tPosLoc = GetUniformLocation(g_tShader, "uPos");
        g_tAtlasLoc = GetUniformLocation(g_tShader, "uAtlas");

        shadersInitialized = true;
    }

    GetContextResources();
}

void SableUI::DestroyDrawables()
{
    void* ctx = SableUI::GetCurrentContext();

    auto it = g_contextResources.find(ctx);
    if (it != g_contextResources.end())
    {
        glDeleteVertexArrays(1, &it->second.rectVAO);
        glDeleteBuffers(1, &it->second.rectVBO);
        glDeleteBuffers(1, &it->second.rectEBO);

        g_contextResources.erase(it);
    }
}

// ============================================================================
// DrawableBase
// ============================================================================
static int s_drawableBaseCount = 0;

SableUI::DrawableBase::DrawableBase()
{
    s_drawableBaseCount++;
    this->uuid = GetUUID();
}

SableUI::DrawableBase::~DrawableBase()
{
    s_drawableBaseCount--;
}

int SableUI::DrawableBase::GetNumInstances()
{
    return s_drawableBaseCount;
}

unsigned int SableUI::DrawableBase::GetUUID()
{
    static unsigned int s_nextUUID = 0;
    return s_nextUUID++;
}

// ============================================================================
// DrawableRect
// ============================================================================
static int s_drawableRectCount = 0;

SableUI::DrawableRect::DrawableRect()
{
    s_drawableRectCount++;
    this->m_zIndex = 0;
}

SableUI::DrawableRect::DrawableRect(SableUI::Rect& r, SableUI::Colour colour)
    : m_colour(colour)
{
    s_drawableRectCount++;
    this->m_rect = r;
    this->m_zIndex = 0;
}

SableUI::DrawableRect::~DrawableRect()
{
    s_drawableRectCount--;
}

int SableUI::DrawableRect::GetNumInstances()
{
    return s_drawableRectCount;
}

void SableUI::DrawableRect::Update(SableUI::Rect& rect, SableUI::Colour colour, float pBSize)
{
    this->m_rect = rect;
    this->m_colour = colour;
}

void SableUI::DrawableRect::Draw(SableUI::RenderTarget* texture, SableUI::ContextResources& res)
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

    /* prevent negative scale */
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform4f(g_rUColourLoc, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f, m_colour.a / 255.0f);
    glUniform1i(g_rUTexBoolLoc, 0);
    glBindVertexArray(res.rectVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// ============================================================================
// DrawableSplitter
// ============================================================================
static int s_drawableSplitterCount = 0;

SableUI::DrawableSplitter::DrawableSplitter()
{
    s_drawableSplitterCount++;
    this->m_zIndex = 1;
}

SableUI::DrawableSplitter::DrawableSplitter(SableUI::Rect& r, SableUI::Colour colour)
    : m_colour(colour)
{
    s_drawableSplitterCount++;
    this->m_zIndex = 999;
    this->m_rect = r;
}

SableUI::DrawableSplitter::~DrawableSplitter()
{
    s_drawableSplitterCount--;
    m_offsets.clear();
}

int SableUI::DrawableSplitter::GetNumInstances()
{
    return s_drawableSplitterCount;
}

void SableUI::DrawableSplitter::Update(SableUI::Rect& rect, SableUI::Colour colour, SableUI::PanelType type,
    float pBSize, const std::vector<int>& segments)
{
    this->m_rect = rect;
    this->m_colour = colour;
    this->m_type = type;
    this->m_bSize = SableUI::f2i(pBSize);
    this->m_offsets = segments;
}

void SableUI::DrawableSplitter::Draw(SableUI::RenderTarget* texture, ContextResources& res)
{

}

// ============================================================================
// DrawableImage
// ============================================================================
static int s_drawableImageCount = 0;

SableUI::DrawableImage::DrawableImage()
{
    s_drawableImageCount++;
    this->m_zIndex = 0;
}

SableUI::DrawableImage::~DrawableImage()
{
    s_drawableImageCount--;
}

int SableUI::DrawableImage::GetNumInstances()
{
    return s_drawableImageCount;
}

void SableUI::DrawableImage::Draw(SableUI::RenderTarget* renderTarget, ContextResources& res)
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

    /* prevent negative scale */
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    m_texture.Bind();

    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform1i(g_rUTexBoolLoc, 1);
    glBindVertexArray(res.rectVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// ============================================================================
// DrawableText
// ============================================================================
static int s_drawableTextCount = 0;

SableUI::DrawableText::DrawableText()
{
    s_drawableTextCount++;
    this->m_zIndex = 0;
}

SableUI::DrawableText::~DrawableText()
{
    s_drawableTextCount--;
}

int SableUI::DrawableText::GetNumInstances()
{
    return s_drawableTextCount;
}

void SableUI::DrawableText::Draw(SableUI::RenderTarget* renderTarget, ContextResources& res)
{
    g_tShader.Use();
    glBindVertexArray(m_text.m_VAO);
    glUniform2f(g_tTargetSizeLoc, static_cast<float>(renderTarget->width), static_cast<float>(renderTarget->height));
    glUniform2f(g_tPosLoc, m_rect.x, m_rect.y + m_rect.h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, GetAtlasTexture());

    glUniform1i(g_tAtlasLoc, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    glDrawElements(GL_TRIANGLES, m_text.indiciesSize, GL_UNSIGNED_INT, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SableUI::DrawWindowBorder(RenderTarget* target)
{
    
}