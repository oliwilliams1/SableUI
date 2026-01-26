#include <glad/glad.h>
#include <SableUI/core/shader.h>
#include <SableUI/generated/shaders.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/text.h>
#include <SableUI/core/window.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/utils.h>
#include <algorithm>
#include <vector>
#include <map>
#include <optional>

using namespace SableUI;

/* rect globals - now per-context */
static std::map<void*, SableUI::ContextResources> g_contextResources;
static GlobalResources g_res{};

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

GlobalResources& SableUI::GetGlobalResources()
{
	return g_res;
}

void SableUI::InitDrawables()
{
	static bool shadersInitialized = false;
	if (!shadersInitialized)
	{
		g_res.s_rect.LoadBasicShaders(rect_vert, rect_frag);
		g_res.s_rect.Use();

		g_res.u_rectColour = GetUniformLocation(g_res.s_rect, "uColour");
		g_res.u_rectRect = GetUniformLocation(g_res.s_rect, "uRect");
		g_res.u_rectRadius = GetUniformLocation(g_res.s_rect, "uRadius");
		g_res.u_rectRealRect = GetUniformLocation(g_res.s_rect, "uRealRect");
		g_res.u_rectTexBool = GetUniformLocation(g_res.s_rect, "uUseTexture");
		g_res.u_rectBorderSize = GetUniformLocation(g_res.s_rect, "uBorderSize");
		g_res.u_rectBorderColour = GetUniformLocation(g_res.s_rect, "uBorderColour");
		glUniform4f(g_res.u_rectColour, 32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f);
		glUniform1i(glGetUniformLocation(g_res.s_rect.m_shaderProgram, "uTexture"), 0);

		g_res.s_text.LoadBasicShaders(text_vert, text_frag);
		g_res.u_textTargetSize = GetUniformLocation(g_res.s_text, "uTargetSize");
		g_res.u_textPos = GetUniformLocation(g_res.s_text, "uPos");
		g_res.u_textAtlas = GetUniformLocation(g_res.s_text, "uAtlas");

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

void DrawableRect::Update(
	const Rect& rect,
	std::optional<Colour> colour,
	float rTL, float rTR,
	float rBL, float rBR,
	std::optional<Colour> borderColour,
	int bT, int bB,
	int bL, int bR,
	bool clipEnabled,
	const Rect& clipRect)
{
	this->m_rect = rect;
	this->m_colour = colour;
	this->m_rTL = rTL;
	this->m_rTR = rTR;
	this->m_rBL = rBL;
	this->m_rBR = rBR;
	this->m_borderColour = borderColour;
	this->m_bT = bT;
	this->m_bB = bB;
	this->m_bL = bL;
	this->m_bR = bR;
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

	g_res.s_rect.Use();
	glUniform4f(g_res.u_rectRect, x, y, w, h);
	glUniform4f(g_res.u_rectRealRect, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
	glUniform4f(g_res.u_rectRadius, m_rTL, m_rTR, m_rBL, m_rBR);
	glUniform4i(g_res.u_rectBorderSize, m_bT, m_bB, m_bL, m_bR);
	if (m_borderColour.has_value())
	{
		Colour bc = m_borderColour.value_or(Colour{ 0, 0, 0, 0 });
		glUniform3f(g_res.u_rectColour, bc.r / 255.0f, bc.g / 255.0f, bc.b / 255.0f);
	}
	Colour c = m_colour.value_or(Colour{ 0, 0, 0, 0 });
	glUniform4f(g_res.u_rectColour, c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
	glUniform1i(g_res.u_rectTexBool, 0);
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
	if (m_type == PanelType::Undef || m_type == PanelType::Base || m_type == PanelType::Root)
		return;

	g_res.s_rect.Use();
	glUniform1i(g_res.u_rectTexBool, 0);
	glUniform4f(g_res.u_rectRadius, 0.0f, 0.0f, 0.0f, 0.0f);
	glUniform4f(g_res.u_rectColour, m_colour.r / 255.0f, m_colour.g / 255.0f, m_colour.b / 255.0f, m_colour.a / 255.0f);

	int startX = std::clamp(m_rect.x, 0, framebuffer->width);
	int startY = std::clamp(m_rect.y, 0, framebuffer->height);
	int boundWidth = std::clamp(m_rect.w, 0, framebuffer->width - startX);
	int boundHeight = std::clamp(m_rect.h, 0, framebuffer->height - startY);

	static auto drawRect = [framebuffer, res](float x, float y, float w, float h) {
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

		glUniform4f(g_res.u_rectRect, normalizedX, normalizedY, normalizedW, normalizedH);
		glUniform4f(g_res.u_rectRealRect, x, y, w, h);
		res.rectObject->AddToDrawStack();
	};

	if (m_type == PanelType::HorizontalSplitter)
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
	else if (m_type == PanelType::VerticalSplitter)
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

void SableUI::DrawableImage::Update(
	Rect& rect,
	float rTL, float rTR,
	float rBL, float rBR,
	std::optional<Colour> borderColour,
	int bT, int bB,
	int bL, int bR,
	bool clipEnabled,
	const Rect& clipRect)
{
	this->m_rect = rect;
	this->m_rTL = rTL;
	this->m_rTR = rTR;
	this->m_rBL = rBL;
	this->m_rBR = rBR;
	this->m_borderColour = borderColour;
	this->m_bT = bT;
	this->m_bB = bB;
	this->m_bL = bL;
	this->m_bR = bR;
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

	g_res.s_rect.Use();
	glUniform4f(g_res.u_rectRect, x, y, w, h);
	glUniform4f(g_res.u_rectRealRect, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
	glUniform4f(g_res.u_rectRadius, m_rTL, m_rTR, m_rBL, m_rBR);
	glUniform4i(g_res.u_rectBorderSize, m_bT, m_bB, m_bL, m_bR);
	if (m_borderColour.has_value())
	{
		Colour bc = m_borderColour.value_or(Colour{ 0, 0, 0, 0 });
		glUniform3f(g_res.u_rectColour, bc.r / 255.0f, bc.g / 255.0f, bc.b / 255.0f);
	}
	glUniform1i(g_res.u_rectTexBool, 1);
	res.rectObject->AddToDrawStack();
}

void SableUI::DrawableImage::RegisterTextureDependancy(BaseComponent* component)
{
	m_texture.RegisterDependancy(component);
}

void SableUI::DrawableImage::DeregisterTextureDependancy(BaseComponent* component)
{
	m_texture.DeregisterDependancy(component);
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
	g_res.s_text.Use();
	glUniform2f(g_res.u_textTargetSize, static_cast<float>(framebuffer->width), static_cast<float>(framebuffer->height));
	glUniform2f(g_res.u_textPos, m_rect.x, m_rect.y + m_rect.h);

	glActiveTexture(GL_TEXTURE0);
	
	BindTextAtlasTexture();

	glUniform1i(g_res.u_textAtlas, 0);

	m_text.m_gpuObject->AddToDrawStack();
}