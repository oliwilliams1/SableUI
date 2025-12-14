#include <map>
#include <algorithm>
#include <SableUI/drawable.h>
#include <SableUI/shader.h>
#include <SableUI/SableUI.h>
#include <SableUI/generated/shaders.h>
#include <SableUI/renderer.h>
#include <vector>
#include <SableUI/console.h>
#include <SableUI/text.h>
#include <SableUI/utils.h>
#include <GL/glew.h>

using namespace SableUI;

/* rect globals - now per-context */
static std::map<void*, SableUI::ContextResources> g_contextResources;

/* shared resources*/
static GLuint g_shaderProgram = 0;
static Shader g_rShader;
static GLuint g_rUColourLoc = 0;
static GLuint g_rURectLoc = 0;
static GLuint g_rUTexBoolLoc = 0;
static GLuint g_rURealRectLoc = 0;
static GLuint g_rURadius = 0;

/* text globals */
static Shader g_tShader;
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

static GLuint GetUniformLocation(Shader shader, const char* uniformName)
{
    shader.Use();
    GLuint location = glGetUniformLocation(shader.m_shaderProgram, uniformName);
    if (location == -1)
    {
        SableUI_Error("Warning: %s uniform not found!", uniformName);
    }
    return location;
}

ContextResources& SableUI::GetContextResources(RendererBackend* backend)
{
    void* ctx = GetCurrentContext_voidType();

    auto it = g_contextResources.find(ctx);
    if (it != g_contextResources.end())
    {
        return it->second;
    }

    ContextResources& resources = g_contextResources[ctx];

    VertexLayout layout;
    layout.Add(VertexFormat::Float2);
    resources.rectObject = backend->CreateGpuObject(
        rectVertices, 
        sizeof(rectVertices) / sizeof(Vertex),
        indices, 
        sizeof(indices) / sizeof(unsigned int), 
        layout
    );

    return resources;
}

void SableUI::InitDrawables()
{
    static bool shadersInitialized = false;
    if (!shadersInitialized)
    {
        g_rShader.LoadBasicShaders(rect_vert, rect_frag);
        g_rShader.Use();
        g_rUColourLoc = GetUniformLocation(g_rShader, "uColour");
        g_rURectLoc = GetUniformLocation(g_rShader, "uRect");
        g_rURadius = GetUniformLocation(g_rShader, "uRadius");
        g_rURealRectLoc = GetUniformLocation(g_rShader, "uRealRect");
        g_rUTexBoolLoc = GetUniformLocation(g_rShader, "uUseTexture");
        glUniform4f(g_rUColourLoc, 32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);
        glUniform1i(glGetUniformLocation(g_rShader.m_shaderProgram, "uTexture"), 0);

        g_tShader.LoadBasicShaders(text_vert, text_frag);
        g_tTargetSizeLoc = GetUniformLocation(g_tShader, "uTargetSize");
        g_tPosLoc = GetUniformLocation(g_tShader, "uPos");
        g_tAtlasLoc = GetUniformLocation(g_tShader, "uAtlas");

        shadersInitialized = true;
    }
}

void SableUI::DestroyDrawables()
{
    void* ctx = SableUI::GetCurrentContext_voidType();

    auto it = g_contextResources.find(ctx);
    if (it != g_contextResources.end())
    {
        auto& res = it->second;
        res.rectObject->m_context->DestroyGpuObject(res.rectObject);
    }

    g_contextResources.clear();
}

// ============================================================================
// DrawableBase
// ============================================================================
static int s_drawableBaseCount = 0;

DrawableBase::DrawableBase()
{
    s_drawableBaseCount++;
    this->uuid = GetUUID();
}

DrawableBase::~DrawableBase()
{
    s_drawableBaseCount--;
}

int DrawableBase::GetNumInstances()
{
    return s_drawableBaseCount;
}

unsigned int DrawableBase::GetUUID()
{
    static unsigned int s_nextUUID = 0;
    return s_nextUUID++;
}

// ============================================================================
// DrawableRect
// ============================================================================
static int s_drawableRectCount = 0;

DrawableRect::DrawableRect()
{
    s_drawableRectCount++;
    this->m_zIndex = 0;
}

DrawableRect::~DrawableRect()
{
    s_drawableRectCount--;
}

int DrawableRect::GetNumInstances()
{
    return s_drawableRectCount;
}

void DrawableRect::Update(const Rect& rect, Colour colour, float borderRadius, bool clipEnabled, const Rect& clipRect)
{
    this->m_rect = rect;
    this->m_colour = colour;
    this->m_borderRadius = borderRadius;
    this->scissorEnabled = clipEnabled;
    this->scissorRect = clipRect;
}

void DrawableRect::Draw(const GpuFramebuffer* framebuffer, ContextResources& res)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (m_rect.x / static_cast<float>(framebuffer->width));
    float y = (m_rect.y / static_cast<float>(framebuffer->height));
    float w = (m_rect.w / static_cast<float>(framebuffer->width));
    float h = (m_rect.h / static_cast<float>(framebuffer->height));

    /* normalise to opengl NDC [0, 1] ->[-1, 1] */
    x = x * 2.0f - 1.0f;
    y = y * 2.0f - 1.0f;
    w *= 2.0f;
    h *= 2.0f;

    /* prevent negative scale */
    w = (std::max)(0.0f, w);
    h = (std::max)(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform4f(g_rURealRectLoc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
    glUniform1f(g_rURadius, m_borderRadius);
    glUniform4f(g_rUColourLoc, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f, m_colour.a / 255.0f);
    glUniform1i(g_rUTexBoolLoc, 0);
    res.rectObject->AddToDrawStack();
}

// ============================================================================
// DrawableSplitter
// ============================================================================
static int s_drawableSplitterCount = 0;

DrawableSplitter::DrawableSplitter()
{
    s_drawableSplitterCount++;
    this->m_zIndex = 1;
}

DrawableSplitter::DrawableSplitter(Rect& r, Colour colour)
    : m_colour(colour)
{
    s_drawableSplitterCount++;
    this->m_zIndex = 999;
    this->m_rect = r;
}

DrawableSplitter::~DrawableSplitter()
{
    s_drawableSplitterCount--;
    m_offsets.clear();
}

int DrawableSplitter::GetNumInstances()
{
    return s_drawableSplitterCount;
}

void DrawableSplitter::Update(Rect& rect, Colour colour, PanelType type,
    float pBSize, const std::vector<int>& segments)
{
    this->m_rect = rect;
    this->m_colour = colour;
    this->m_type = type;
    this->m_bSize = SableUI::f2i(pBSize);
    this->m_offsets = segments;
}

void DrawableSplitter::Draw(const GpuFramebuffer* framebuffer, ContextResources& res)
{
    if (m_type == PanelType::Undef || m_type == PanelType::BASE || m_type == PanelType::ROOTNODE)
        return;

    g_rShader.Use();
    glUniform1i(g_rUTexBoolLoc, 0);
    glUniform1f(g_rURadius, 0.0f);
    glUniform4f(g_rUColourLoc, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f, m_colour.a / 255.0f);

    int startX = std::clamp(m_rect.x, 0, framebuffer->width);
    int startY = std::clamp(m_rect.y, 0, framebuffer->height);
    int boundWidth = std::clamp(m_rect.w, 0, framebuffer->width - startX);
    int boundHeight = std::clamp(m_rect.h, 0, framebuffer->height - startY);

    auto drawRect = [&](float x, float y, float w, float h) {
        /* normalise from texture bounds to [0, 1] */
        float normalizedX = (x / static_cast<float>(framebuffer->width));
        float normalizedY = (y / static_cast<float>(framebuffer->height));
        float normalizedW = (w / static_cast<float>(framebuffer->width));
        float normalizedH = (h / static_cast<float>(framebuffer->height));

        /* normalise to opengl NDC [0, 1] -> [-1, 1] */
        normalizedX = normalizedX * 2.0f - 1.0f;
        normalizedY = normalizedY * 2.0f - 1.0f;
        normalizedW *= 2.0f;
        normalizedH *= 2.0f;

        /* prevent negative scale */
        normalizedW = (std::max)(0.0f, normalizedW);
        normalizedH = (std::max)(0.0f, normalizedH);

        /* invert y axis */
        normalizedY *= -1.0f;
        normalizedH *= -1.0f;

        glUniform4f(g_rURectLoc, normalizedX, normalizedY, normalizedW, normalizedH);
        glUniform4f(g_rURealRectLoc, x, y, w, h);
        res.rectObject->AddToDrawStack();
    };

    if (m_type == PanelType::HORIZONTAL)
    {
        for (int offset : m_offsets)
        {
            float drawX = startX + static_cast<float>(offset) - m_bSize;
            float drawWidth = m_bSize * 2.0f;

            if (drawX + drawWidth >= 0 && drawX < framebuffer->width)
            {
                drawRect(drawX, static_cast<float>(startY), drawWidth, static_cast<float>(boundHeight));
            }
        }
    }
    else if (m_type == PanelType::VERTICAL)
    {
        for (int offset : m_offsets)
        {
            float drawY = startY + static_cast<float>(offset) - m_bSize;
            float drawHeight = m_bSize * 2.0f;

            if (drawY + drawHeight >= 0 && drawY < framebuffer->height)
            {
                drawRect(static_cast<float>(startX), drawY, static_cast<float>(boundWidth), drawHeight);
            }
        }
    }
}

// ============================================================================
// DrawableImage
// ============================================================================
static int s_drawableImageCount = 0;

DrawableImage::DrawableImage()
{
    s_drawableImageCount++;
    this->m_zIndex = 0;
}

DrawableImage::~DrawableImage()
{
    s_drawableImageCount--;
}

int DrawableImage::GetNumInstances()
{
    return s_drawableImageCount;
}

void SableUI::DrawableImage::Update(Rect& rect, float borderRadius, bool clipEnabled, const Rect& clipRect)
{
    this->m_rect = rect;
    this->m_borderRadius = borderRadius;
    this->scissorEnabled = clipEnabled;
    this->scissorRect = clipRect;
}

void DrawableImage::Draw(const GpuFramebuffer* framebuffer, ContextResources& res)
{
    /* normalise from texture bounds to [0, 1] */
    float x = (m_rect.x / static_cast<float>(framebuffer->width));
    float y = (m_rect.y / static_cast<float>(framebuffer->height));
    float w = (m_rect.w / static_cast<float>(framebuffer->width));
    float h = (m_rect.h / static_cast<float>(framebuffer->height));

    /* normalise to opengl NDC [0, 1] ->[-1, 1] */
    x = x * 2.0f - 1.0f;
    y = y * 2.0f - 1.0f;
    w *= 2.0f;
    h *= 2.0f;

    /* prevent negative scale */
    w = (std::max)(0.0f, w);
    h = (std::max)(0.0f, h);

    /* invert y axis */
    y *= -1.0f;
    h *= -1.0f;

    m_texture.Bind();

    g_rShader.Use();
    glUniform4f(g_rURectLoc, x, y, w, h);
    glUniform4f(g_rURealRectLoc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
    glUniform1f(g_rURadius, m_borderRadius);
    glUniform1i(g_rUTexBoolLoc, 1);
    res.rectObject->AddToDrawStack();
}

// ============================================================================
// DrawableText
// ============================================================================
static int s_drawableTextCount = 0;

DrawableText::DrawableText()
{
    s_drawableTextCount++;
    this->m_zIndex = 0;
}

DrawableText::~DrawableText()
{
    s_drawableTextCount--;
}

int DrawableText::GetNumInstances()
{
    return s_drawableTextCount;
}

void SableUI::DrawableText::Update(Rect& rect, bool clipEnabled, const Rect& clipRect)
{
    this->m_rect = rect;
    this->scissorEnabled = clipEnabled;
    this->scissorRect = clipRect;
};

void DrawableText::Draw(const GpuFramebuffer* framebuffer, ContextResources& res)
{
    g_tShader.Use();
    glUniform2f(g_tTargetSizeLoc, static_cast<float>(framebuffer->width), static_cast<float>(framebuffer->height));
    glUniform2f(g_tPosLoc, m_rect.x, m_rect.y + m_rect.h);

    glActiveTexture(GL_TEXTURE0);
    
    BindTextAtlasTexture();

    glUniform1i(g_tAtlasLoc, 0);

    m_text.m_gpuObject->AddToDrawStack();
}