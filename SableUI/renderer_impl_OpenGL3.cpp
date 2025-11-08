#include "SableUI/renderer.h"
#include <algorithm>
#include <set>

#include "SableUI/console.h"
#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Renderer"
using namespace SableUI;

static GLenum BlendFactorToOpenGLEnum(BlendFactor factor)
{
	switch (factor)
	{
	case BlendFactor::Zero:						return GL_ZERO;
	case BlendFactor::One:						return GL_ONE;
	case BlendFactor::SrcColor:					return GL_SRC_COLOR;
	case BlendFactor::OneMinusSrcColor:			return GL_ONE_MINUS_SRC_COLOR;
	case BlendFactor::DstColor:					return GL_DST_COLOR;
	case BlendFactor::OneMinusDstColor:			return GL_ONE_MINUS_DST_COLOR;
	case BlendFactor::SrcAlpha:					return GL_SRC_ALPHA;
	case BlendFactor::OneMinusSrcAlpha:			return GL_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::DstAlpha:					return GL_DST_ALPHA;
	case BlendFactor::OneMinusDstAlpha:			return GL_ONE_MINUS_DST_ALPHA;
	case BlendFactor::ConstantColor:			return GL_CONSTANT_COLOR;
	case BlendFactor::OneMinusConstantColor:	return GL_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::ConstantAlpha:			return GL_CONSTANT_ALPHA;
	case BlendFactor::OneMinusConstantAlpha:	return GL_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::SrcAlphaSaturate:			return GL_SRC_ALPHA_SATURATE;
	default:
		SableUI_Runtime_Error("Unknown blend factor, %i", (int)factor);
		return GL_ONE;
	}
}

void sRenderer::InitOpenGL()
{
	SableUI_Log("Using OpenGL backend");

	// init after window is cleared
	GLenum res = glewInit();
	if (GLEW_OK != res)
	{
		SableUI_Runtime_Error("Could not initialize GLEW: %s", glewGetErrorString(res));
	}
}

void sRenderer::SetBlending(bool enabled)
{
	if (enabled) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
}

void sRenderer::SetBlendFunction(BlendFactor src, BlendFactor dst)
{
	glBlendFunc(BlendFactorToOpenGLEnum(src), BlendFactorToOpenGLEnum(dst));
}

void sRenderer::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void sRenderer::Flush()
{
	glFlush();
}

void sRenderer::Viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void sRenderer::CheckErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		SableUI_Error("OpenGL error: %s", gluErrorString(err));
	}
}

// ============================================================================
// Drawables
// ============================================================================
void sRenderer::ClearDrawableStack()
{
	m_drawStack.clear();
}

void sRenderer::ClearDrawable(DrawableBase* drawable)
{
	for (DrawableBase* d : m_drawStack)
		if (d == drawable)
			m_drawStack.erase(std::remove(m_drawStack.begin(), m_drawStack.end(), d), m_drawStack.end());
}

void sRenderer::Draw(DrawableBase* drawable)
{
	m_drawStack.push_back(drawable);
}

void sRenderer::StartDirectDraw()
{
	m_directDraw = true;
}

void sRenderer::DirectDrawRect(const Rect& rect, const Colour& colour)
{
	DrawableRect dr;
	dr.m_rect = rect;
	dr.m_colour = colour;
	m_drawStack.push_back(&dr);
}

void sRenderer::EndDirectDraw()
{
	m_directDraw = false;
	Flush();
}

void sRenderer::Draw()
{
	if (m_drawStack.size() == 0) return;

	if (m_directDraw)
		SableUI_Warn("Direct draw is enabled when drawing normally, forgot to end direct draw?");

	if (m_renderTarget.targetType == RenderTargetType::Texture) m_renderTarget.Bind();

	ContextResources& res = GetContextResources();

	std::sort(m_drawStack.begin(), m_drawStack.end(), [](const DrawableBase* a, const DrawableBase* b) {
		return a->m_zIndex < b->m_zIndex;
	});

	std::set<unsigned int> drawnUUIDs;

	// iterate through queue and draw all types of drawables
	for (const auto& drawable : m_drawStack)
	{
		if (drawable)
		{
			if (drawnUUIDs.find(drawable->uuid) != drawnUUIDs.end()) continue;
			drawnUUIDs.insert(drawable->uuid);
			drawable->Draw(&m_renderTarget, res);
		}
	}

	// draw window border after queue is drawn
	// DrawWindowBorder(&m_renderTarget);

	m_drawStack.clear();

	Flush();
}

// ============================================================================
// Render Target
// ============================================================================
SableUI::RenderTarget::RenderTarget(int width, int height)
	: width(width), height(height)
{
	InitTexture();
}

SableUI::RenderTarget::~RenderTarget()
{
	glDeleteTextures(1, &m_textureID);
}

void SableUI::RenderTarget::InitTexture()
{
	if (targetType == RenderTargetType::Window)
	{
		SableUI_Warn("Cannot create GPU texture for texture target of window");
		return;
	};

	if (m_textureID == 0)
	{
		glGenTextures(1, &m_textureID);
	}

	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SableUI::RenderTarget::Resize(int w, int h)
{
	bool u = false;

	if (w != width || h != height)
		u = true;

	width = w;
	height = h;

	if (u && targetType != RenderTargetType::Window) Update();
}

void SableUI::RenderTarget::Update() const
{
	if (targetType == RenderTargetType::Window)
	{
		SableUI_Warn("Cannot update GPU texture for texture target of window");
		return;
	};

	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void SableUI::RenderTarget::Bind() const
{
	if (targetType == RenderTargetType::Window)
	{
		SableUI_Warn("Cannot bind GPU texture for texture target of window");
		return;
	};

	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

// ============================================================================
// Texture2D
// ============================================================================
void GpuTexture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void GpuTexture2D::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GpuTexture2D::SetData(const uint8_t* pixels, int width, int height, int channels)
{
	if (m_textureID == 0)
		glGenTextures(1, (GLuint*)&m_textureID);

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	GLenum internalFormat = GL_RGB8;
	GLenum format = GL_RGB;

	if (channels == 4)
	{
		internalFormat = GL_RGBA8;
		format = GL_RGBA;
	}
	else if (channels == 3)
	{
		internalFormat = GL_RGB8;
		format = GL_RGB;
	}
	else
	{
		SableUI_Warn("Unsupported channel count in SetData: %d", channels);
		internalFormat = GL_RGB8;
		format = GL_RGB;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_width = width;
	m_height = height;
}

GpuTexture2D::~GpuTexture2D()
{
	glDeleteTextures(1, &m_textureID);
}

// ============================================================================
// Texture2DArray
// ============================================================================
void GpuTexture2DArray::Bind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
}

void GpuTexture2DArray::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void SableUI::GpuTexture2DArray::Init(int width, int height, int depth)
{
	if (m_textureID == 0)
		glGenTextures(1, &m_textureID);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, width, height, depth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void SableUI::GpuTexture2DArray::Resize(int newDepth)
{
	if (newDepth <= m_depth)
	{
		SableUI_Warn("Trying to resize texture array to a smaller or same depth (old: %i, new: %i)",
			m_depth, newDepth);
		return;
	}

	GLuint newTextureArray = 0;
	glGenTextures(1, &newTextureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, newTextureArray);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, m_width, m_height, newDepth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* copy old texture to new, with new depth */
	GLuint oldAtlasTextureArray = m_textureID;
	glCopyImageSubData(oldAtlasTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		newTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		m_width, m_height, m_depth);

	glDeleteTextures(1, &oldAtlasTextureArray);
	m_textureID = newTextureArray;
	m_depth = newDepth;
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void SableUI::GpuTexture2DArray::SubImage(int xOffset, int yOffset, int zOffset,
	int width, int height, int depth, const uint8_t* pixels)
{
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, xOffset, yOffset, zOffset,
		width, height, depth, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void SableUI::GpuTexture2DArray::CopyImageSubData(const GpuTexture2DArray& src,
	int srcX, int srcY, int srcZ, int dstX,
	int dstY, int dstZ, int width, int height, int depth)
{
	glCopyImageSubData(src.m_textureID, GL_TEXTURE_2D_ARRAY, 0,
		srcX, srcY, srcZ, m_textureID, GL_TEXTURE_2D_ARRAY, 0,\
		dstX, dstY, dstZ, width, height, depth);
}

SableUI::GpuTexture2DArray::GpuTexture2DArray(GpuTexture2DArray&& other) noexcept
	: m_textureID(other.m_textureID)
	, m_width(other.m_width)
	, m_height(other.m_height)
	, m_depth(other.m_depth)
{
	other.m_textureID = 0;
	other.m_width = 0;
	other.m_height = 0;
	other.m_depth = 0;
}

GpuTexture2DArray& SableUI::GpuTexture2DArray::operator=(GpuTexture2DArray&& other) noexcept
{
	if (this != &other)
	{
		if (m_textureID != 0)
		{
			glDeleteTextures(1, &m_textureID);
		}

		m_textureID = other.m_textureID;
		m_width = other.m_width;
		m_height = other.m_height;
		m_depth = other.m_depth;

		other.m_textureID = 0;
		other.m_width = 0;
		other.m_height = 0;
		other.m_depth = 0;
	}
	return *this;
}

SableUI::GpuTexture2DArray::~GpuTexture2DArray()
{
	glDeleteTextures(1, &m_textureID);
}