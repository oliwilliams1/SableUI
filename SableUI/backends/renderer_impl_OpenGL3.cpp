#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <SableUI/renderer/renderer.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/drawable.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/shader.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/gpu_texture.h>
#include <SableUI/renderer/gpu_object.h>
#include <SableUI/renderer/gpu_framebuffer.h>
#include <SableUI/renderer/resource_handle.h>
#include <variant>

#include <SableUI/utils/console.h>
#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "Renderer"

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace SableUI;

// ============================================================================
// OpenGL Backend
// ============================================================================
class OpenGL3Backend : public RendererBackend
{
public:
	OpenGL3Backend() { Initialise(); }
	~OpenGL3Backend();
	void Initialise() override;
	void Clear(float r, float g, float b, float a) override;
	void Viewport(int x, int y, int width, int height) override;
	void SetBlending(bool enabled) override;
	void SetBlendFunction(BlendFactor src, BlendFactor dst) override;
	void CheckErrors() override;

	uint32_t CreateUniformBuffer(size_t size, const void* initialData = nullptr) override;
	void DestroyUniformBuffer(uint32_t ubo) override;
	void BindUniformBufferBase(uint32_t binding, uint32_t ubo) override;

	void ExecuteCommandBuffer() override;

	GpuObject* CreateGpuObject(
		const void* vertices, uint32_t numVertices,
		const uint32_t* indices, uint32_t numIndices,
		const VertexLayout& layout) override;

	void DestroyGpuObject(GpuObject* obj) override;
	GpuObjectMetadata& GetMeshMetadata(uint32_t handle) override;

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
	struct OpenGLMesh
	{
		GLuint vao = 0, vbo = 0, ebo = 0;
	};
	std::unordered_map<uint32_t, OpenGLMesh> m_meshes;
	std::unordered_map<uint32_t, GpuObjectMetadata> m_meshMetadata;

	friend class OpenGLCommandExecutor;
	OpenGLMesh& GetMesh(uint32_t handle) { return m_meshes[handle]; }
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

// ============================================================================
// Abstract Type Conversions
// ============================================================================
static GLenum BlendFactorToGL(BlendFactor factor)
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

static GLenum TextureInterpolationToGL(TextureInterpolation interp)
{
	switch (interp)
	{
	case TextureInterpolation::Nearest: return GL_NEAREST;
	case TextureInterpolation::Linear: return GL_LINEAR;
	default: return GL_NEAREST;
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

static inline GLenum TextureFormatToGLFormat(TextureFormat format)
{
	switch (format)
	{
	case SableUI::TextureFormat::RGBA8:		return GL_RGBA;
	case SableUI::TextureFormat::RGB8:		return GL_RGB;
	case SableUI::TextureFormat::RG8:		return GL_RG;
	case SableUI::TextureFormat::R8:		return GL_RED;
	case SableUI::TextureFormat::Undefined:	return GL_RGB;
	default:								return GL_RGB;
	}
}

static inline GLenum TextureFormatToGLInternalFormat(TextureFormat format)
{
	switch (format)
	{
	case SableUI::TextureFormat::RGBA8:		return GL_RGBA8;
	case SableUI::TextureFormat::RGB8:		return GL_RGB8;
	case SableUI::TextureFormat::RG8:		return GL_RG8;
	case SableUI::TextureFormat::R8:		return GL_R8;
	case SableUI::TextureFormat::Undefined:	return GL_RGB8;
	default:								return GL_RGB8;
	}
}

static inline GLenum TextureTypeToGL(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture2D:			return GL_TEXTURE_2D;
	case TextureType::Texture2DArray:		return GL_TEXTURE_2D_ARRAY;
	default:								return GL_TEXTURE_2D;
	}
}

static inline void VertexFormatToGL(VertexFormat format, bool& isInteger, int& count, GLenum& type)
{
	switch (format)
	{
	case VertexFormat::Float1:	count = 1; type = GL_FLOAT;								break;
	case VertexFormat::Float2:	count = 2; type = GL_FLOAT;								break;
	case VertexFormat::Float3:	count = 3; type = GL_FLOAT;								break;
	case VertexFormat::Float4:	count = 4; type = GL_FLOAT;								break;
	case VertexFormat::UInt1:	count = 1; type = GL_UNSIGNED_INT;	isInteger = true;	break;
	case VertexFormat::UInt2:	count = 2; type = GL_UNSIGNED_INT;	isInteger = true;	break;
	case VertexFormat::UInt3:	count = 3; type = GL_UNSIGNED_INT;	isInteger = true;	break;
	case VertexFormat::UInt4:	count = 4; type = GL_UNSIGNED_INT;	isInteger = true;	break;
	case VertexFormat::Int1:	count = 1; type = GL_INT;			isInteger = true;	break;
	case VertexFormat::Int2:	count = 2; type = GL_INT;			isInteger = true;	break;
	case VertexFormat::Int3:	count = 3; type = GL_INT;			isInteger = true;	break;
	case VertexFormat::Int4:	count = 4; type = GL_INT;			isInteger = true;	break;
	default: break;
	}
}

// ============================================================================
// OpenGL3Backend Implementations
// ============================================================================
bool gladInitialised = false;
void OpenGL3Backend::Initialise()
{
	if (!gladInitialised)
	{
		SableUI_Log("Using OpenGL backend");

		// init after window is cleared
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
		{
			SableUI_Runtime_Error("Failed to initialize GLAD");
		}
	
		gladInitialised = true;
	}

	m_commandBuffer.SetAllocator(&m_resourceAllocator);

	CommandBuffer tempCB = CreateSecondaryCommandBuffer();
	SetupContextResources(tempCB, this);

	m_executor = CommandBufferExecutor::Create(Backend::OpenGL, &GetGlobalResources(), &GetContextResources(this), this);

	m_executor->Execute(tempCB);
}

OpenGL3Backend::~OpenGL3Backend()
{
	SableMemory::SB_delete(m_executor);
}

void OpenGL3Backend::SetBlending(bool enabled)
{
	if (enabled) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
}

void OpenGL3Backend::SetBlendFunction(BlendFactor src, BlendFactor dst)
{
	glBlendFunc(BlendFactorToGL(src), BlendFactorToGL(dst));
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

uint32_t OpenGL3Backend::CreateUniformBuffer(size_t size, const void* initialData)
{
	GLuint ubo = 0;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, initialData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return ubo;
}

void OpenGL3Backend::DestroyUniformBuffer(uint32_t ubo)
{
	GLuint buffer = ubo;
	glDeleteBuffers(1, &buffer);
}

void OpenGL3Backend::BindUniformBufferBase(uint32_t binding, uint32_t ubo)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
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

	RectDrawData data{};

	float invW = 1.0f / float(windowSize.w);
	float invH = 1.0f / float(windowSize.h);

	float x = destRect.x * invW * 2.0f - 1.0f;
	float y = destRect.y * invH * 2.0f - 1.0f;
	float w = destRect.w * invW * 2.0f;
	float h = destRect.h * invH * 2.0f;

	y *= -1.0f;
	h *= -1.0f;

	data.rect[0] = x;
	data.rect[1] = y;
	data.rect[2] = w;
	data.rect[3] = h;

	data.realRect[0] = sourceRect.x;
	data.realRect[1] = sourceRect.y;
	data.realRect[2] = sourceRect.w;
	data.realRect[3] = sourceRect.h;

	data.radius[0] = 0.0f;
	data.radius[1] = 0.0f;
	data.radius[2] = 0.0f;
	data.radius[3] = 0.0f;

	data.borderSize[0] = 0;
	data.borderSize[1] = 0;
	data.borderSize[2] = 0;
	data.borderSize[3] = 0;

	data.colour[0] = 1.0f;
	data.colour[1] = 1.0f;
	data.colour[2] = 1.0f;
	data.colour[3] = 1.0f;

	data.useTexture = 1;

	source->GetColorAttachments()[0].Bind(0);

	g_res.s_rect.Use();

	glBindBuffer(GL_UNIFORM_BUFFER, g_res.ubo_rect);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RectDrawData), &data);

	//c_res.rectObject->AddToDrawStack();
}

// ============================================================================
// Drawables
// ============================================================================
static int s_numGpuObjects = 0;
GpuObject* OpenGL3Backend::CreateGpuObject(
	const void* vertices, uint32_t numVertices,
	const uint32_t* indices, uint32_t numIndices,
	const VertexLayout& layout)
{
	GpuObject* obj = SableMemory::SB_new<GpuObject>();
	obj->context = this;
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

		VertexFormatToGL(attr.format, isInteger, count, type);

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

	GpuObjectMetadata metadata;
	metadata.vertexCount = numVertices;
	metadata.indexCount = numIndices;
	m_meshMetadata[handle] = metadata;

	obj->handle = handle;
	return obj;
}

void OpenGL3Backend::DestroyGpuObject(GpuObject* obj)
{
	auto it = m_meshes.find(obj->handle);
	if (it == m_meshes.end()) return;

	OpenGLMesh& mesh = it->second;

	if (mesh.vao != 0) glDeleteVertexArrays(1, &mesh.vao);
	if (mesh.vbo != 0) glDeleteBuffers(1, &mesh.vbo);
	if (mesh.ebo != 0) glDeleteBuffers(1, &mesh.ebo);

	m_meshes.erase(it);
	FreeHandle(obj->handle);
	s_numGpuObjects--;

	obj->context = nullptr;
	SableMemory::SB_delete(obj);
}

GpuObjectMetadata& OpenGL3Backend::GetMeshMetadata(uint32_t handle)
{
	return m_meshMetadata[handle];
}

void OpenGL3Backend::ExecuteCommandBuffer()
{
	m_executor->Execute(m_commandBuffer);
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
// Gpu Textures
// ============================================================================
void GpuTexture2D::Bind(uint32_t slot) const
{
	if (slot > 15)
		SableUI_Error("A maximum of 15 texture units is supported for compatibility");

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, handle);
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
	if (handle == 0)
		glGenTextures(1, &handle);

	glBindTexture(GL_TEXTURE_2D, handle);

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
	if (handle == 0)
		glGenTextures(1, &handle);

	glBindTexture(GL_TEXTURE_2D, handle);

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
	if (handle != 0)
		glDeleteTextures(1, &handle);
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
void GpuTexture2DArray::Bind(uint32_t slot) const
{
	if (slot > 15)
		SableUI_Error("A maximum of 15 texture units is supported for compatibility");

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
}

void GpuTexture2DArray::Unbind(uint32_t slot) const
{
	if (slot > 15)
		SableUI_Error("A maximum of 15 texture units is supported for compatibility");

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GpuTexture2DArray::Init(int width, int height, int depth)
{
	if (handle == 0)
		glGenTextures(1, &handle);

	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
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

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, m_width, m_height, newDepth);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* copy old texture to new, with new depth */
	GLuint oldAtlasTextureArray = handle;
	glCopyImageSubData(oldAtlasTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		newTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
		m_width, m_height, m_depth);

	glDeleteTextures(1, &oldAtlasTextureArray);
	handle = newTextureArray;
	m_depth = newDepth;
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GpuTexture2DArray::SubImage(int xOffset, int yOffset, int zOffset,
	int width, int height, int depth, const uint8_t* pixels)
{
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, xOffset, yOffset, zOffset,
		width, height, depth, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void GpuTexture2DArray::CopyImageSubData(const GpuTexture2DArray& src,
	int srcX, int srcY, int srcZ, int dstX,
	int dstY, int dstZ, int width, int height, int depth)
{
	glCopyImageSubData(src.handle, GL_TEXTURE_2D_ARRAY, 0,
		srcX, srcY, srcZ, handle, GL_TEXTURE_2D_ARRAY, 0,
		dstX, dstY, dstZ, width, height, depth);
}

GpuTexture2DArray::~GpuTexture2DArray()
{
	glDeleteTextures(1, &handle);
}

// ===========================================================================
// Gpu Object
// ============================================================================
GpuObject::GpuObject()
{
	s_numGpuObjects++;
}

GpuObject::~GpuObject()
{
	if (context)
		context->DestroyGpuObject(this);
}

int GpuObject::GetNumInstances()
{
	return s_numGpuObjects;
}

// ============================================================================
// OpenGL Command Buffer Executor
// ============================================================================
class OpenGLCommandExecutor : public CommandBufferExecutor
{
public:
	OpenGLCommandExecutor(GlobalResources* globalRes, ContextResources* contextRes, OpenGL3Backend* backend)
		: m_globalRes(globalRes), m_contextRes(contextRes), m_backend(backend) {};

	void Execute(const CommandBuffer& cmdBuffer) override
	{
		for (const Command& cmd : cmdBuffer.GetCommands())
		{
			switch (cmd.type)
			{
			case CommandType::SetPipeline:
				ExecuteSetPipeline(std::get<SetPipelineCmd>(cmd.data));
				break;

			case CommandType::SetBlendState:
				ExecuteSetBlendState(std::get<SetBlendStateCmd>(cmd.data));
				break;

			case CommandType::SetScissor:
				ExecuteSetScissor(std::get<SetScissorCmd>(cmd.data));
				break;

			case CommandType::DisableScissor:
				glDisable(GL_SCISSOR_TEST);
				break;

			case CommandType::CreateGpuObject:
				ExecuteCreateGpuObject(std::get<CreateGpuObjectCmd>(cmd.data), cmd.inlineData);
				break;
			
			case CommandType::BindGpuObject:
				ExecuteBindGpuObject(std::get<BindGpuObjectCmd>(cmd.data));
				break;

			case CommandType::DestroyGpuObject:
				ExecuteDestroyGpuObject(std::get<DestroyGpuObjectCmd>(cmd.data));
				break;

			case CommandType::BindUniformBuffer:
				ExecuteBindUniformBuffer(std::get<BindUniformBufferCmd>(cmd.data));
				break;

			case CommandType::BindTexture:
				ExecuteBindTexture(std::get<BindTextureCmd>(cmd.data));
				break;

			case CommandType::UpdateUniformBuffer:
				ExecuteUpdateUniformBuffer(std::get<UpdateUniformBufferCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::DrawGpuObject:
				ExecuteDrawGpuObject(std::get<DrawGpuObjectCmd>(cmd.data));
				break;

			case CommandType::DrawIndexed:
				ExecuteDrawIndexed(std::get<DrawIndexedCmd>(cmd.data));
				break;

			case CommandType::Draw:
				ExecuteDraw(std::get<DrawCmd>(cmd.data));
				break;

			case CommandType::Clear:
				ExecuteClear(std::get<ClearCmd>(cmd.data));
				break;

			case CommandType::BeginRenderPass:
				ExecuteBeginRenderPass(std::get<BeginRenderPassCmd>(cmd.data));
				break;

			case CommandType::EndRenderPass:
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				break;

			case CommandType::BlitFramebuffer:
				ExecuteBlitFramebuffer(std::get<BlitFramebufferCmd>(cmd.data));
				break;

			case CommandType::BlitToScreen:
				ExecuteBlitToScreen(std::get<BlitToScreenCmd>(cmd.data));
				break;

			default:
				SableUI_Error("Unknown command type");
				break;
			}
		}
	}

private:
	std::unordered_map<ResourceHandle, uint32_t> m_gpuHandles;
	GlobalResources* m_globalRes;
	ContextResources* m_contextRes;
	OpenGL3Backend* m_backend;

	void ExecuteSetPipeline(const SetPipelineCmd& cmd)
	{
		switch (cmd.pipeline)
		{
		case PipelineType::Rect:
			m_globalRes->s_rect.Use();
			break;
		case PipelineType::Text:
			m_globalRes->s_text.Use();
			break;
		case PipelineType::Image:
			m_globalRes->s_rect.Use();
			break;
		}
	}

	void ExecuteSetBlendState(const SetBlendStateCmd& cmd)
	{
		if (cmd.enabled)
		{
			glEnable(GL_BLEND);
			glBlendFunc(BlendFactorToGL(cmd.srcFactor), BlendFactorToGL(cmd.dstFactor));
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void ExecuteSetScissor(const SetScissorCmd& cmd)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(cmd.x, cmd.y, cmd.width, cmd.height);
	}

	void ExecuteBindUniformBuffer(const BindUniformBufferCmd& cmd)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, cmd.binding, cmd.ubo);
	}

	void ExecuteBindTexture(const BindTextureCmd& cmd)
	{
		glActiveTexture(GL_TEXTURE0 + cmd.slot);
		glBindTexture(TextureTypeToGL(cmd.type), cmd.handle);
	}

	void ExecuteCreateGpuObject(
		const CreateGpuObjectCmd& cmd,
		const std::vector<uint8_t>& data)
	{
		size_t vertexDataSize = static_cast<size_t>(cmd.numVertices) * cmd.layout.stride;
		const void* vertexData = data.data();
		const uint32_t* indexData = nullptr;

		if (cmd.numIndices > 0)
			indexData = reinterpret_cast<const uint32_t*>(data.data() + vertexDataSize);

		GpuObject* obj = m_backend->CreateGpuObject(
			vertexData, cmd.numVertices,
			indexData, cmd.numIndices,
			cmd.layout
		);

		m_gpuHandles[cmd.handle] = obj->handle;
	}

	uint32_t GetGpuHandle(ResourceHandle cpuHandle)
	{
		auto it = m_gpuHandles.find(cpuHandle);
		if (it == m_gpuHandles.end())
		{
			SableUI_Runtime_Error("Invalid CPU handle, resource not created yet");
			return 0;
		}
		return it->second;
	}

	void ExecuteBindGpuObject(const BindGpuObjectCmd& cmd)
	{
		uint32_t gpuHandle = GetGpuHandle(cmd.handle);
		OpenGL3Backend::OpenGLMesh& mesh = m_backend->GetMesh(gpuHandle);
		glBindVertexArray(mesh.vao);
	}

	void ExecuteDestroyGpuObject(const DestroyGpuObjectCmd& cmd)
	{
		auto it = m_gpuHandles.find(cmd.handle);
		if (it == m_gpuHandles.end())
		{
			SableUI_Warn("Attempted to destroy GPU object that doesn't exist (CPU handle: %u)",	cmd.handle.index);
			return;
		}

		uint32_t gpuHandle = it->second;

		auto meshIt = m_backend->m_meshes.find(gpuHandle);
		if (meshIt == m_backend->m_meshes.end())
		{
			SableUI_Error("GPU handle exists in mapping but mesh doesn't exist (GPU handle: %u)", gpuHandle);
			m_gpuHandles.erase(it);
			return;
		}

		OpenGL3Backend::OpenGLMesh& mesh = meshIt->second;

		if (mesh.vao != 0) glDeleteVertexArrays(1, &mesh.vao);
		if (mesh.vbo != 0) glDeleteBuffers(1, &mesh.vbo);
		if (mesh.ebo != 0) glDeleteBuffers(1, &mesh.ebo);

		m_backend->m_meshes.erase(meshIt);
		m_backend->FreeHandle(gpuHandle);
		m_gpuHandles.erase(it);
	}

	void ExecuteUpdateUniformBuffer(const UpdateUniformBufferCmd& cmd, const std::vector<uint8_t>& data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, cmd.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, cmd.offset, cmd.size, data.data());
	}

	void ExecuteDrawGpuObject(const DrawGpuObjectCmd& cmd)
	{
		uint32_t gpuHandle = GetGpuHandle(cmd.handle);
		OpenGL3Backend::OpenGLMesh& mesh = m_backend->GetMesh(gpuHandle);
		GpuObjectMetadata& meshMetadata = m_backend->GetMeshMetadata(gpuHandle);
		glBindVertexArray(mesh.vao);
		
		if (cmd.instanceCount > 1)
		{
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				meshMetadata.indexCount,
				GL_UNSIGNED_INT,
				0,
				cmd.instanceCount,
				0
			);
		}
		else
		{
			glDrawElements(
				GL_TRIANGLES,
				meshMetadata.indexCount,
				GL_UNSIGNED_INT,
				0
			);
		}
	}

	void ExecuteDrawIndexed(const DrawIndexedCmd& cmd)
	{
		if (cmd.instanceCount > 1)
		{
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				cmd.indexCount,
				GL_UNSIGNED_INT,
				(void*)(cmd.firstIndex * sizeof(uint32_t)),
				cmd.instanceCount,
				cmd.vertexOffset
			);
		}
		else
		{
			glDrawElements(
				GL_TRIANGLES,
				cmd.indexCount,
				GL_UNSIGNED_INT,
				(void*)(cmd.firstIndex * sizeof(uint32_t))
			);
		}
	}

	void ExecuteDraw(const DrawCmd& cmd)
	{
		if (cmd.instanceCount > 1)
			glDrawArraysInstanced(GL_TRIANGLES, cmd.firstVertex, cmd.vertexCount, cmd.instanceCount);
		else
			glDrawArrays(GL_TRIANGLES, cmd.firstVertex, cmd.vertexCount);
	}

	void ExecuteClear(const ClearCmd& cmd)
	{
		glClearColor(cmd.r, cmd.g, cmd.b, cmd.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void ExecuteBeginRenderPass(const BeginRenderPassCmd& cmd)
	{
		if (!cmd.framebuffer->isWindowSurface)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, cmd.framebuffer->GetHandle());
			glViewport(0, 0, cmd.framebuffer->GetColorAttachments()[0].GetWidth(),
				cmd.framebuffer->GetColorAttachments()[0].GetHeight());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void ExecuteBlitFramebuffer(const BlitFramebufferCmd& cmd)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, cmd.srcFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cmd.dstFBO);
		glBlitFramebuffer(
			cmd.srcX0, cmd.srcY0, cmd.srcX1, cmd.srcY1,
			cmd.dstX0, cmd.dstY0, cmd.dstX1, cmd.dstY1,
			GL_COLOR_BUFFER_BIT,
			TextureInterpolationToGL(cmd.filter)
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ExecuteBlitToScreen(const BlitToScreenCmd& cmd)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, cmd.framebuffer->GetHandle());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, cmd.framebuffer->width, cmd.framebuffer->height,
			0, 0, cmd.framebuffer->width, cmd.framebuffer->height,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

SableUI::CommandBufferExecutor* SableUI::CommandBufferExecutor::Create(
	Backend backend,
	GlobalResources* globalRes,
	ContextResources* contextRes,
	RendererBackend* renderer)
{
	if (OpenGL3Backend* oglBackend = dynamic_cast<OpenGL3Backend*>(renderer))
	{
		switch (backend)
		{
		case SableUI::Backend::OpenGL:
			return SableMemory::SB_new<OpenGLCommandExecutor>(globalRes, contextRes, oglBackend);
			break;
		default:
			SableUI_Error("Resorting to OpenGL command buffer executor");
			return SableMemory::SB_new<OpenGLCommandExecutor>(globalRes, contextRes, oglBackend);
			break;
		}
	}
	else
	{
		SableUI_Runtime_Error("OpenGL command buffer executer initialised with a non OpenGL renderer");
		return nullptr;
	}
}