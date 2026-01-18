#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <set>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <SableUI/core/renderer.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/drawable.h>
#include <SableUI/utils/utils.h>

#include <SableUI/utils/console.h>
#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Renderer"
using namespace SableUI;

struct OpenGLMesh
{
	GLuint vao = 0, vbo = 0, ebo = 0;
};

class OpenGL3Backend : public RendererBackend
{
public:
	OpenGL3Backend() { Initialise(); }
	void Initialise() override;
	void Clear(float r, float g, float b, float a) override;
	void Viewport(int x, int y, int width, int height) override;
	void SetBlending(bool enabled) override;
	void SetBlendFunction(BlendFactor src, BlendFactor dst) override;

	void CheckErrors() override;

	void ClearDrawableStack() override;
	void ClearDrawable(const DrawableBase* drawable) override;

	void AddToDrawStack(DrawableBase* drawable) override;
	void AddToDrawStack(const GpuObject* obj) override;
	bool Draw(const GpuFramebuffer* framebuffer) override;

	GpuObject* CreateGpuObject(
		const void* vertices, uint32_t numVertices,
		const uint32_t* indices, uint32_t numIndices,
		const VertexLayout& layout) override;
	void DestroyGpuObject(GpuObject* obj) override;
	void BeginRenderPass(const GpuFramebuffer* fbo) override;
	void EndRenderPass() override;
	void BlitToScreen(GpuFramebuffer* source,
		TextureInterpolation interpolation) override;
	void BlitToScreenWithRects(
		GpuFramebuffer* source,
		const Rect& sourceRect,
		const Rect& destRect,
		TextureInterpolation interpolation) override;
	void BlitToFramebuffer(
		GpuFramebuffer* source,
		GpuFramebuffer* target,
		Rect sourceRect, Rect destRect,
		TextureInterpolation interpolation) override;
	void DrawToScreen(
		GpuFramebuffer* source,
		const Rect& sourceRect,
		const Rect& destRect,
		const ivec2& windowSize) override;

private:
	std::unordered_map<uint32_t, OpenGLMesh> m_meshes;
};

RendererBackend* SableUI::RendererBackend::Create(Backend backend)
{
	switch (backend)
	{
	case SableUI::Backend::OpenGL:
		return SableMemory::SB_new<OpenGL3Backend>();
		break;
	default:
		SableUI_Error("Resorting to OpenGL");
		return SableMemory::SB_new<OpenGL3Backend>();
		break;
	}
}

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

static GLenum TextureInterpolationToOpenGLEnum(TextureInterpolation interpolation)
{
	switch (interpolation)
	{
	case TextureInterpolation::Nearest:	return GL_NEAREST;
	case TextureInterpolation::Linear:	return GL_LINEAR;
	default:
		SableUI_Runtime_Error("Unknown texture interpolation, %i", (int)interpolation);
		return GL_NEAREST;
	}
}

bool gladInitialised = false;
void OpenGL3Backend::Initialise()
{
	if (gladInitialised)
		return;

	SableUI_Log("Using OpenGL backend");

	// init after window is cleared
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        SableUI_Runtime_Error("Failed to initialize GLAD");
    }
	
	gladInitialised = true;
}

void OpenGL3Backend::SetBlending(bool enabled)
{
	if (enabled) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
}

void OpenGL3Backend::SetBlendFunction(BlendFactor src, BlendFactor dst)
{
	glBlendFunc(BlendFactorToOpenGLEnum(src), BlendFactorToOpenGLEnum(dst));
}

void OpenGL3Backend::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGL3Backend::Viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void OpenGL3Backend::CheckErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		SableUI_Error("OpenGL error: %d", err);
	}
}

void OpenGL3Backend::BeginRenderPass(const GpuFramebuffer* fbo)
{
	if (!fbo->isWindowSurface)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->GetHandle());
		glViewport(0, 0, fbo->GetColorAttachments()[0].GetWidth(),
			fbo->GetColorAttachments()[0].GetHeight());
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void OpenGL3Backend::EndRenderPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL3Backend::BlitToScreen(
	GpuFramebuffer* source,
	TextureInterpolation interpolation)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source->GetHandle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, source->width, source->height,
		0, 0, source->width, source->height,
		GL_COLOR_BUFFER_BIT,
		TextureInterpolationToOpenGLEnum(interpolation));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL3Backend::BlitToScreenWithRects(
	GpuFramebuffer* source,
	const Rect& sourceRect,
	const Rect& destRect,
	TextureInterpolation interpolation)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source->GetHandle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(
		sourceRect.x, sourceRect.y,
		sourceRect.x + sourceRect.w, sourceRect.y + sourceRect.h,
		destRect.x, destRect.y,
		destRect.x + destRect.w, destRect.y + destRect.h,
		GL_COLOR_BUFFER_BIT,
		TextureInterpolationToOpenGLEnum(interpolation)
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL3Backend::BlitToFramebuffer(
	GpuFramebuffer* source, GpuFramebuffer* target,
	Rect sourceRect, Rect destRect,
	TextureInterpolation interpolation)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, source->GetHandle());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->GetHandle());
	glBlitFramebuffer(
		sourceRect.x, sourceRect.y,
		sourceRect.x + sourceRect.width, sourceRect.y + sourceRect.height,
		destRect.x, destRect.y,
		destRect.x + destRect.width, destRect.y + destRect.height,
		GL_COLOR_BUFFER_BIT,
		TextureInterpolationToOpenGLEnum(interpolation));
}

void OpenGL3Backend::DrawToScreen(
	GpuFramebuffer* source,
	const Rect& sourceRect,
	const Rect& destRect,
	const ivec2& windowSize)
{
	GlobalResources& g_res = SableUI::GetGlobalResources();
	ContextResources& c_res = SableUI::GetContextResources(this);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Viewport(0, 0, windowSize.w, windowSize.h);

	float invW = 1.0f / float(windowSize.w);
	float invH = 1.0f / float(windowSize.h);

	float x_ndc = destRect.x * invW * 2.0f - 1.0f;
	float y_ndc = destRect.y * invH * 2.0f - 1.0f;

	float w_ndc = destRect.w * invW * 2.0f;
	float h_ndc = destRect.h * invH * 2.0f;

	source->GetColorAttachments()[0].Bind(0);

	g_res.s_rect.Use();
	glUniform4f(g_res.u_rectRect, x_ndc, y_ndc, w_ndc, h_ndc);
	glUniform4f(g_res.u_rectRealRect,
		sourceRect.x, sourceRect.y,
		sourceRect.w, sourceRect.h);

	glUniform1f(g_res.u_rectRadius, 0.0f);
	glUniform1i(g_res.u_rectTexBool, 1);

	c_res.rectObject->AddToDrawStack();
}

// ============================================================================
// Drawables
// ============================================================================
void OpenGL3Backend::ClearDrawableStack()
{
	m_drawStack.clear();
}

void OpenGL3Backend::ClearDrawable(const DrawableBase* drawable)
{
	if (!drawable) return;
	for (DrawableBase* d : m_drawStack)
		if (d == drawable)
			m_drawStack.erase(std::remove(m_drawStack.begin(), m_drawStack.end(), d), m_drawStack.end());
}

static int s_numGpuObjects = 0;
GpuObject* OpenGL3Backend::CreateGpuObject(
	const void* vertices, uint32_t numVertices,
	const uint32_t* indices, uint32_t numIndices,
	const VertexLayout& layout)
{
	GpuObject* obj = SableMemory::SB_new<GpuObject>();
	obj->m_context = this;
	obj->numVertices = numVertices;
	obj->numIndices = numIndices;
	obj->layout = layout;

	uint32_t handle = AllocateHandle();
	m_meshes[handle] = OpenGLMesh();
	OpenGLMesh& GLmesh = m_meshes[handle];

	glGenVertexArrays(1, &GLmesh.vao);
	glBindVertexArray(GLmesh.vao);

	glGenBuffers(1, &GLmesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, GLmesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, numVertices * layout.stride, vertices, GL_STATIC_DRAW);

	if (indices && numIndices > 0)
	{
		glGenBuffers(1, &GLmesh.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLmesh.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	uint32_t attrIndex = 0;
	for (const auto& attr : layout.attributes)
	{
		glEnableVertexAttribArray(attrIndex);
		GLenum type = GL_FLOAT;
		GLint count = 1;
		GLboolean normalized = attr.normalised ? GL_TRUE : GL_FALSE;
		bool isInteger = false;

		switch (attr.format)
		{
		case VertexFormat::Float1: count = 1; type = GL_FLOAT; break;
		case VertexFormat::Float2: count = 2; type = GL_FLOAT; break;
		case VertexFormat::Float3: count = 3; type = GL_FLOAT; break;
		case VertexFormat::Float4: count = 4; type = GL_FLOAT; break;
		case VertexFormat::UInt1: count = 1; type = GL_UNSIGNED_INT; isInteger = true; break;
		case VertexFormat::UInt2: count = 2; type = GL_UNSIGNED_INT; isInteger = true; break;
		case VertexFormat::UInt3: count = 3; type = GL_UNSIGNED_INT; isInteger = true; break;
		case VertexFormat::UInt4: count = 4; type = GL_UNSIGNED_INT; isInteger = true; break;
		case VertexFormat::Int1: count = 1; type = GL_INT; isInteger = true; break;
		case VertexFormat::Int2: count = 2; type = GL_INT; isInteger = true; break;
		case VertexFormat::Int3: count = 3; type = GL_INT; isInteger = true; break;
		case VertexFormat::Int4: count = 4; type = GL_INT; isInteger = true; break;
		default: break;
		}

		if (isInteger)
		{
			glVertexAttribIPointer(
				attrIndex,
				count,
				type,
				layout.stride,
				reinterpret_cast<void*>(attr.offset)
			);
		}
		else
		{
			glVertexAttribPointer(
				attrIndex,
				count,
				type,
				normalized,
				layout.stride,
				reinterpret_cast<void*>(attr.offset)
			);
		}
		attrIndex++;
	}

	glBindVertexArray(0);

	obj->m_handle = handle;
	return obj;
}

void OpenGL3Backend::DestroyGpuObject(GpuObject* obj)
{
	auto it = m_meshes.find(obj->m_handle);
	if (it == m_meshes.end()) return;

	OpenGLMesh& mesh = it->second;

	if (mesh.vao != 0) glDeleteVertexArrays(1, &mesh.vao);
	if (mesh.vbo != 0) glDeleteBuffers(1, &mesh.vbo);
	if (mesh.ebo != 0) glDeleteBuffers(1, &mesh.ebo);

	m_meshes.erase(it);
	FreeHandle(obj->m_handle);
	s_numGpuObjects--;

	obj->m_context = nullptr;
	SableMemory::SB_delete(obj);
}

bool OpenGL3Backend::Draw(const GpuFramebuffer* framebuffer)
{
	if (m_drawStack.empty()) return false;
	if (!framebuffer) return false;

	ContextResources& res = GetContextResources(this);

	std::set<unsigned int> drawnUUIDs;

	bool scissorActive = false;
	Rect currentScissor;

	for (DrawableBase* drawable : m_drawStack)
	{
		if (!drawable) continue;
		if (!drawnUUIDs.insert(drawable->uuid).second) continue;

		bool wantScissor = drawable->scissorEnabled;

		if (wantScissor)
		{
			Rect newScissor = drawable->scissorRect;

			newScissor.y = framebuffer->height - (newScissor.y + newScissor.h);

			if (!scissorActive || currentScissor != newScissor)
			{
				if (!scissorActive)
					glEnable(GL_SCISSOR_TEST);

				int finalY = std::max(0, newScissor.y);

				glScissor(newScissor.x, finalY, newScissor.w, newScissor.h);
				currentScissor = newScissor;
				scissorActive = true;
			}
		}
		else
		{
			if (scissorActive)
			{
				glDisable(GL_SCISSOR_TEST);
				scissorActive = false;
			}
		}

		drawable->Draw(framebuffer, res);

		if (drawable->orphan)
			SableMemory::SB_delete(drawable);
	}

	if (scissorActive)
		glDisable(GL_SCISSOR_TEST);

	m_drawStack.clear();
	return true;
}

void OpenGL3Backend::AddToDrawStack(DrawableBase* drawable)
{
	m_drawStack.push_back(drawable);
}

void OpenGL3Backend::AddToDrawStack(const GpuObject* obj)
{
	OpenGLMesh& mesh = m_meshes[obj->m_handle];
	if (mesh.vao == 0)
	{
		SableUI_Error("Invalid GPU object");
		return;
	}

	glBindVertexArray(mesh.vao);
	glDrawElements(GL_TRIANGLES, obj->numIndices, GL_UNSIGNED_INT, 0);
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
static inline GLenum TextureFormatToGLFormat(TextureFormat format)
{
	switch (format)
	{
	case SableUI::TextureFormat::RGBA8:
		return GL_RGBA;
	case SableUI::TextureFormat::RGB8:
		return GL_RGB;
	case SableUI::TextureFormat::RG8:
		return GL_RG;
	case SableUI::TextureFormat::R8:
		return GL_RED;
	case SableUI::TextureFormat::Undefined:
		return GL_RGB;
	default:
		return GL_RGB;
	}
}

static inline GLenum TextureFormatToGLInternalFormat(TextureFormat format)
{
	switch (format)
	{
	case SableUI::TextureFormat::RGBA8:
		return GL_RGBA8;
	case SableUI::TextureFormat::RGB8:
		return GL_RGB8;
	case SableUI::TextureFormat::RG8:
		return GL_RG8;
	case SableUI::TextureFormat::R8:
		return GL_R8;
	case SableUI::TextureFormat::Undefined:
		return GL_RGB8;
	default:
		return GL_RGB8;
	}
}

void GpuTexture2D::Bind(uint32_t slot) const
{
	if (slot > 15)
		SableUI_Error("A maximum of 15 texture units is supported for compatibility");
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void GpuTexture2D::Unbind(uint32_t slot) const
{
	if (slot > 15)
		SableUI_Error("A maximum of 15 texture units is supported for compatibility");
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GpuTexture2D::CreateStorage(int width, int height, TextureFormat format, TextureUsage usage)
{
	if (m_textureID == 0)
		glGenTextures(1, &m_textureID);

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	GLenum internalFormat = TextureFormatToGLInternalFormat(format);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
		TextureFormatToGLFormat(format), GL_UNSIGNED_BYTE, nullptr);

	if (usage == TextureUsage::RenderTarget)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_width = width;
	m_height = height;
	m_format = format;
	m_usage = usage;
}

void GpuTexture2D::SetData(const uint8_t* pixels, int width, int height, TextureFormat p_format)
{
	if (m_textureID == 0)
		glGenTextures(1, &m_textureID);

	glBindTexture(GL_TEXTURE_2D, m_textureID);

	GLenum internalFormat = TextureFormatToGLInternalFormat(p_format);
	GLenum format = TextureFormatToGLFormat(p_format);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_width = width;
	m_height = height;
	m_format = p_format;
}

GpuTexture2D::~GpuTexture2D()
{
	if (m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}

// ============================================================================
// Framebuffer
// ============================================================================
void GpuFramebuffer::AttachColour(GpuTexture2D* texture, int slot)
{
	if (isWindowSurface)
	{
		SableUI_Error("Cannot add a colour attachment to a window surface");
		return;
	}
	if (slot < 0 || slot >= 8)
	{
		SableUI_Error("Color attachment slot must be between 0 and 7");
		return;
	}

	if (m_colorAttachments.size() <= static_cast<size_t>(slot))
	{
		m_colorAttachments.resize(slot + 1);
	}

	m_colorAttachments[slot] = std::move(*texture);
}

void GpuFramebuffer::AttachDepthStencil(GpuTexture2D* texture)
{
	m_depthStencilAttachment = std::move(*texture);
}

void GpuFramebuffer::Bake()
{
	if (isWindowSurface)
	{
		SableUI_Error("Cannot bake a window-surface framebuffer");
		return;
	}

	if (m_handle == 0)
		glGenFramebuffers(1, &m_handle);

	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);

	std::vector<GLenum> drawBuffers;
	for (size_t i = 0; i < m_colorAttachments.size(); ++i)
	{
		if (m_colorAttachments[i].GetHandle() != 0)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				GL_TEXTURE_2D, m_colorAttachments[i].GetHandle(), 0);
			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
		}
	}

	if (!drawBuffers.empty())
	{
		glDrawBuffers(drawBuffers.size(), drawBuffers.data());
	}

	if (m_depthStencilAttachment.GetHandle() != 0)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_TEXTURE_2D, m_depthStencilAttachment.GetHandle(), 0);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		SableUI_Error("Framebuffer is not complete! Status: 0x%x", status);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GpuFramebuffer::SetSize(int p_width, int p_height)
{
	if (width == p_width && width == p_height)
		return;

	width = p_width;
	height = p_height;

	for (auto& texture : m_colorAttachments)
	{
		if (texture.GetHandle() != 0)
		{
			texture.CreateStorage(width, height, texture.GetFormat(), texture.GetUsage());
		}
	}

	if (m_depthStencilAttachment.GetHandle() != 0)
	{
		m_depthStencilAttachment.CreateStorage(width, height, m_depthStencilAttachment.GetFormat(), TextureUsage::RenderTarget);
	}

	if (m_handle != 0)
		Bake();
}

GpuFramebuffer::~GpuFramebuffer()
{
	if (m_handle != 0)
		glDeleteFramebuffers(1, &m_handle);
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

void GpuTexture2DArray::Init(int width, int height, int depth)
{
	if (m_textureID == 0)
		glGenTextures(1, &m_textureID);

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, width, height, depth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GpuTexture2DArray::Resize(int newDepth)
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

	// Changed from GL_RGB8 to GL_R8
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, m_width, m_height, newDepth);

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

void GpuTexture2DArray::SubImage(int xOffset, int yOffset, int zOffset,
	int width, int height, int depth, const uint8_t* pixels)
{
	// Changed from GL_RGB to GL_RED
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, xOffset, yOffset, zOffset,
		width, height, depth, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void GpuTexture2DArray::CopyImageSubData(const GpuTexture2DArray& src,
	int srcX, int srcY, int srcZ, int dstX,
	int dstY, int dstZ, int width, int height, int depth)
{
	glCopyImageSubData(src.m_textureID, GL_TEXTURE_2D_ARRAY, 0,
		srcX, srcY, srcZ, m_textureID, GL_TEXTURE_2D_ARRAY, 0, \
		dstX, dstY, dstZ, width, height, depth);
}

GpuTexture2DArray::GpuTexture2DArray(GpuTexture2DArray&& other) noexcept
	: m_textureID(other.m_textureID),
	m_width(other.m_width),
	m_height(other.m_height),
	m_depth(other.m_depth)
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

GpuTexture2DArray::~GpuTexture2DArray()
{
	glDeleteTextures(1, &m_textureID);
}

// ===========================================================================
// Gpu Object
// ============================================================================
void GpuObject::AddToDrawStack() const
{
	m_context->AddToDrawStack(this);
}

GpuObject::GpuObject()
{
	s_numGpuObjects++;
}

GpuObject::~GpuObject()
{
	if (m_context)
		m_context->DestroyGpuObject(this);
}

int GpuObject::GetNumInstances()
{
	return s_numGpuObjects;
}
