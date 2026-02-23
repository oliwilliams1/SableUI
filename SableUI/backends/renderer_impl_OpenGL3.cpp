#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <SableUI/renderer/renderer.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/shader.h>
#include <SableUI/types/renderer_types.h>
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
	void CheckErrors() override;

	void ExecuteCommandBuffer() override;

private:
	struct OpenGLMesh
	{
		GLuint vao = 0, vbo = 0, ebo = 0;
	};

	friend class OpenGLCommandExecutor;
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

void OpenGL3Backend::CheckErrors()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		SableUI_Error("OpenGL error: %d", err);
	}
}

void OpenGL3Backend::ExecuteCommandBuffer()
{
	m_commandBuffer.DebugPrintAndClear();
	m_executor->Execute(m_commandBuffer);
}

// ============================================================================
// OpenGL Command Buffer Executor
// ============================================================================
class OpenGLCommandExecutor : public CommandBufferExecutor
{
public:
	OpenGLCommandExecutor(GlobalResources* globalRes, ContextResources* contextRes,
		OpenGL3Backend* backend)
		: m_globalRes(globalRes), m_contextRes(contextRes), m_backend(backend) {}

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

			case CommandType::SetViewport:
				ExecuteSetViewport(std::get<SetViewportCmd>(cmd.data));
				break;

			case CommandType::BindUniformBuffer:
				ExecuteBindUniformBuffer(std::get<BindUniformBufferCmd>(cmd.data));
				break;

			case CommandType::BindTexture:
				ExecuteBindTexture(std::get<BindTextureCmd>(cmd.data));
				break;

			case CommandType::BindGpuObject:
				ExecuteBindGpuObject(std::get<BindGpuObjectCmd>(cmd.data));
				break;

			case CommandType::BindFramebuffer:
				ExecuteBindFramebuffer(std::get<BindFramebufferCmd>(cmd.data));
				break;

			case CommandType::CreateGpuObject:
				ExecuteCreateGpuObject(std::get<CreateGpuObjectCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::DestroyGpuObject:
				ExecuteDestroyGpuObject(std::get<DestroyGpuObjectCmd>(cmd.data));
				break;

			case CommandType::CreateTexture2D:
				ExecuteCreateTexture2D(std::get<CreateTexture2DCmd>(cmd.data));
				break;

			case CommandType::CreateTexture2DArray:
				ExecuteCreateTexture2DArray(std::get<CreateTexture2DArrayCmd>(cmd.data));
				break;

			case CommandType::ResizeTexture2DArray:
				ExecuteResizeTexture2DArray(std::get<ResizeTexture2DArrayCmd>(cmd.data));
				break;

			case CommandType::SubImageTexture2DArray:
				ExecuteSubImageTexture2DArray(std::get<SubImageTexture2DArrayCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::CopyImageDataTexture2DArray:
				ExecuteCopyImageDataTexture2DArray(std::get<CopyImageDataTexture2DArrayCmd>(cmd.data));
				break;

			case CommandType::SetDataTexture2D:
				ExecuteSetTextureData(std::get<SetTextureDataCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::CreateStorageTexture2D:
				ExecuteCreateTextureStorage(std::get<CreateTextureStorageCmd>(cmd.data));
				break;

			case CommandType::DestroyTexture:
				ExecuteDestroyTexture(std::get<DestroyTextureCmd>(cmd.data));
				break;

			case CommandType::CreateFramebuffer:
				ExecuteCreateFramebuffer(std::get<CreateFramebufferCmd>(cmd.data));
				break;

			case CommandType::AttachColourTexture:
				ExecuteAttachColorTexture(std::get<AttachColorTextureCmd>(cmd.data));
				break;

			case CommandType::AttachDepthStencilTexture:
				ExecuteAttachDepthStencilTexture(std::get<AttachDepthStencilTextureCmd>(cmd.data));
				break;

			case CommandType::BakeFramebuffer:
				ExecuteBakeFramebuffer(std::get<BakeFramebufferCmd>(cmd.data));
				break;

			case CommandType::SetFramebufferSize:
				ExecuteSetFramebufferSize(std::get<SetFramebufferSizeCmd>(cmd.data));
				break;

			case CommandType::DestroyFramebuffer:
				ExecuteDestroyFramebuffer(std::get<DestroyFramebufferCmd>(cmd.data));
				break;

			case CommandType::CreateUniformBuffer:
				ExecuteCreateUniformBuffer(std::get<CreateUniformBufferCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::UpdateUniformBuffer:
				ExecuteUpdateUniformBuffer(std::get<UpdateUniformBufferCmd>(cmd.data), cmd.inlineData);
				break;

			case CommandType::DestroyUniformBuffer:
				ExecuteDestroyUniformBuffer(std::get<DestroyUniformBufferCmd>(cmd.data));
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
				SableUI_Error("Unknown command type: %d", static_cast<int>(cmd.type));
				break;
			}
		}
	}

private:
	std::unordered_map<ResourceHandle, OpenGL3Backend::OpenGLMesh> m_gpuObjects;
	std::unordered_map<ResourceHandle, GLuint> m_textures;
	std::unordered_map<ResourceHandle, GLuint> m_framebuffers;
	std::unordered_map<ResourceHandle, GLuint> m_uniformBuffers;

	std::unordered_map<ResourceHandle, GpuObjectMetadata> m_meshMetadata;
	std::unordered_map<ResourceHandle, TextureMetadata> m_textureMetadata;
	std::unordered_map<ResourceHandle, FramebufferMetadata> m_framebufferMetadata;

	GlobalResources* m_globalRes;
	ContextResources* m_contextRes;
	OpenGL3Backend* m_backend;

	// ============================================================================
	// Pipeline state
	// ============================================================================

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

	void ExecuteSetViewport(const SetViewportCmd& cmd)
	{
		glViewport(cmd.x, cmd.y, cmd.width, cmd.height);
	}

	// ============================================================================
	// Resource binding
	// ============================================================================
	
	void ExecuteBindUniformBuffer(const BindUniformBufferCmd& cmd)
	{
		auto it = m_uniformBuffers.find(cmd.buffer);
		if (it == m_uniformBuffers.end())
		{
			SableUI_Error("Attempting to bind non-existent uniform buffer");
			return;
		}
		glBindBufferBase(GL_UNIFORM_BUFFER, cmd.binding, it->second);
	}

	void ExecuteBindTexture(const BindTextureCmd& cmd)
	{
		auto it = m_textures.find(cmd.texture);
		if (it == m_textures.end())
		{
			SableUI_Error("Attempting to bind non-existent texture");
			return;
		}

		auto& metadata = m_textureMetadata[cmd.texture];
		GLenum target = (metadata.type == TextureType::Texture2DArray)
			? GL_TEXTURE_2D_ARRAY
			: GL_TEXTURE_2D;

		glActiveTexture(GL_TEXTURE0 + cmd.slot);
		glBindTexture(target, it->second);
	}

	void ExecuteBindGpuObject(const BindGpuObjectCmd& cmd)
	{
		auto it = m_gpuObjects.find(cmd.handle);
		if (it == m_gpuObjects.end())
		{
			SableUI_Error("Attempting to bind non-existent GPU object");
			return;
		}
		glBindVertexArray(it->second.vao);
	}

	void ExecuteBindFramebuffer(const BindFramebufferCmd& cmd)
	{
		auto it = m_framebuffers.find(cmd.framebuffer);
		if (it == m_framebuffers.end())
		{
			SableUI_Error("Attempting to bind non-existent framebuffer");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.framebuffer];
		if (metadata.isWindowSurface)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, it->second);
		}
	}

	// ============================================================================
	// Texture management
	// ============================================================================

	void ExecuteCreateTexture2D(const CreateTexture2DCmd& cmd)
	{
		GLuint texID;
		glGenTextures(1, &texID);
		m_textures[cmd.handle] = texID;

		TextureMetadata metadata;
		metadata.width = cmd.width;
		metadata.height = cmd.height;
		metadata.format = cmd.format;
		metadata.usage = cmd.usage;
		metadata.type = TextureType::Texture2D;
		m_textureMetadata[cmd.handle] = metadata;

		if (cmd.width > 0 && cmd.height > 0)
		{
			glBindTexture(GL_TEXTURE_2D, texID);

			GLenum internalFormat = TextureFormatToGLInternalFormat(cmd.format);
			GLenum format = TextureFormatToGLFormat(cmd.format);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cmd.width, cmd.height,
				0, format, GL_UNSIGNED_BYTE, nullptr);

			if (cmd.usage == TextureUsage::RenderTarget)
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
		}
	}

	void ExecuteCreateTexture2DArray(const CreateTexture2DArrayCmd& cmd)
	{
		GLuint texID;
		glGenTextures(1, &texID);
		m_textures[cmd.handle] = texID;

		TextureMetadata metadata;
		metadata.width = cmd.width;
		metadata.height = cmd.height;
		metadata.depth = cmd.depth;
		metadata.format = cmd.format;
		metadata.usage = cmd.usage;
		metadata.type = TextureType::Texture2DArray;
		m_textureMetadata[cmd.handle] = metadata;

		if (cmd.width > 0 && cmd.height > 0 && cmd.depth > 0)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, texID);

			GLenum internalFormat = TextureFormatToGLInternalFormat(cmd.format);
			GLenum format = TextureFormatToGLFormat(cmd.format);

			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, cmd.width, cmd.height, cmd.depth);

			if (cmd.usage == TextureUsage::RenderTarget)
			{
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}

	void ExecuteResizeTexture2DArray(const ResizeTexture2DArrayCmd& cmd)
	{
		TextureMetadata& metadata = m_textureMetadata[cmd.handle];

		GLuint newTexID;
		glGenTextures(1, &newTexID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, newTexID);

		GLenum internalFormat = TextureFormatToGLInternalFormat(metadata.format);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, metadata.width, metadata.height, cmd.newDepth);

		if (metadata.usage == TextureUsage::RenderTarget)
		{
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLuint oldAtlasTextureArray = m_textures[cmd.handle];
		int copyDepth = std::min(metadata.depth, cmd.newDepth);

		glCopyImageSubData(oldAtlasTextureArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
			newTexID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
			metadata.width, metadata.height, copyDepth);

		metadata.depth = cmd.newDepth;

		glDeleteTextures(1, &oldAtlasTextureArray);
		m_textures[cmd.handle] = newTexID;
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	void ExecuteSubImageTexture2DArray(const SubImageTexture2DArrayCmd& cmd, const std::vector<uint8_t>& data)
	{
		GLuint texID = m_textures[cmd.handle];
		if (texID == 0) return;

		glBindTexture(GL_TEXTURE_2D_ARRAY, texID);

		GLenum format = TextureFormatToGLFormat(cmd.format);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY, 0,
			cmd.xOffset, cmd.yOffset, cmd.zOffset,
			cmd.width, cmd.height, cmd.depth,
			format, GL_UNSIGNED_BYTE,
			data.data()
		);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	void ExecuteCopyImageDataTexture2DArray(const CopyImageDataTexture2DArrayCmd& cmd)
	{
		GLuint src = m_textures[cmd.src];
		GLuint dst = m_textures[cmd.dst];

		glCopyImageSubData(src, GL_TEXTURE_2D_ARRAY, 0,
			cmd.srcX, cmd.srcY, cmd.srcZ, dst, GL_TEXTURE_2D_ARRAY, 0,
			cmd.dstX, cmd.dstY, cmd.dstZ, cmd.width, cmd.height, cmd.depth);
	}

	void ExecuteSetTextureData(const SetTextureDataCmd& cmd,
		const std::vector<uint8_t>& data)
	{
		auto it = m_textures.find(cmd.texture);
		if (it == m_textures.end())
		{
			SableUI_Error("Attempting to set data on non-existent texture");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, it->second);

		GLenum internalFormat = TextureFormatToGLInternalFormat(cmd.format);
		GLenum format = TextureFormatToGLFormat(cmd.format);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cmd.width, cmd.height,
			0, format, GL_UNSIGNED_BYTE, data.data());

		auto& metadata = m_textureMetadata[cmd.texture];
		metadata.width = cmd.width;
		metadata.height = cmd.height;
		metadata.format = cmd.format;
	}

	void ExecuteCreateTextureStorage(const CreateTextureStorageCmd& cmd)
	{
		auto it = m_textures.find(cmd.texture);
		if (it == m_textures.end())
		{
			SableUI_Error("Attempting to create storage for non-existent texture");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, it->second);

		GLenum internalFormat = TextureFormatToGLInternalFormat(cmd.format);
		GLenum format = TextureFormatToGLFormat(cmd.format);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cmd.width, cmd.height,
			0, format, GL_UNSIGNED_BYTE, nullptr);

		auto& metadata = m_textureMetadata[cmd.texture];
		metadata.width = cmd.width;
		metadata.height = cmd.height;
		metadata.format = cmd.format;
		metadata.usage = cmd.usage;
	}

	void ExecuteDestroyTexture(const DestroyTextureCmd& cmd)
	{
		auto it = m_textures.find(cmd.handle);
		if (it == m_textures.end())
		{
			SableUI_Warn("Attempting to destroy non-existent texture");
			return;
		}

		GLuint texID = it->second;
		glDeleteTextures(1, &texID);

		m_textures.erase(it);
		m_textureMetadata.erase(cmd.handle);
	}

	// ============================================================================
	// Framebuffer management
	// ============================================================================

	void ExecuteCreateFramebuffer(const CreateFramebufferCmd& cmd)
	{
		if (cmd.isWindowSurface)
		{
			m_framebuffers[cmd.handle] = 0;

			FramebufferMetadata metadata;
			metadata.width = cmd.width;
			metadata.height = cmd.height;
			metadata.isWindowSurface = true;
			m_framebufferMetadata[cmd.handle] = metadata;
		}
		else
		{
			GLuint fboID;
			glGenFramebuffers(1, &fboID);
			m_framebuffers[cmd.handle] = fboID;

			FramebufferMetadata metadata;
			metadata.width = cmd.width;
			metadata.height = cmd.height;
			metadata.isWindowSurface = false;
			m_framebufferMetadata[cmd.handle] = metadata;
		}
	}

	void ExecuteAttachColorTexture(const AttachColorTextureCmd& cmd)
	{
		auto fboIt = m_framebuffers.find(cmd.framebuffer);
		auto texIt = m_textures.find(cmd.texture);

		if (fboIt == m_framebuffers.end() || texIt == m_textures.end())
		{
			SableUI_Error("Invalid framebuffer or texture handle in AttachColourTexture");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.framebuffer];
		if (metadata.isWindowSurface)
		{
			SableUI_Error("Cannot attach texture to window surface framebuffer");
			return;
		}

		if (metadata.colorAttachments.size() <= static_cast<size_t>(cmd.slot))
		{
			metadata.colorAttachments.resize(cmd.slot + 1);
		}
		metadata.colorAttachments[cmd.slot] = cmd.texture;
	}

	void ExecuteAttachDepthStencilTexture(const AttachDepthStencilTextureCmd& cmd)
	{
		auto& metadata = m_framebufferMetadata[cmd.framebuffer];
		if (metadata.isWindowSurface)
		{
			SableUI_Error("Cannot attach depth/stencil to window surface framebuffer");
			return;
		}

		metadata.depthStencilAttachment = cmd.texture;
	}

	void ExecuteBakeFramebuffer(const BakeFramebufferCmd& cmd)
	{
		auto fboIt = m_framebuffers.find(cmd.framebuffer);
		if (fboIt == m_framebuffers.end())
		{
			SableUI_Error("Attempting to bake non-existent framebuffer");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.framebuffer];
		if (metadata.isWindowSurface)
		{
			SableUI_Warn("Cannot bake window surface framebuffer");
			return;
		}

		GLuint fboID = fboIt->second;
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);

		std::vector<GLenum> drawBuffers;
		for (size_t i = 0; i < metadata.colorAttachments.size(); ++i)
		{
			if (metadata.colorAttachments[i].IsValid())
			{
				auto texIt = m_textures.find(metadata.colorAttachments[i]);
				if (texIt != m_textures.end())
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + i,
						GL_TEXTURE_2D,
						texIt->second,
						0);
					drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
				}
			}
		}

		if (!drawBuffers.empty())
		{
			glDrawBuffers(drawBuffers.size(), drawBuffers.data());
		}

		if (metadata.depthStencilAttachment.IsValid())
		{
			auto texIt = m_textures.find(metadata.depthStencilAttachment);
			if (texIt != m_textures.end())
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER,
					GL_DEPTH_STENCIL_ATTACHMENT,
					GL_TEXTURE_2D,
					texIt->second,
					0);
			}
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			SableUI_Error("Framebuffer is not complete! Status: 0x%x", status);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ExecuteSetFramebufferSize(const SetFramebufferSizeCmd& cmd)
	{
		auto& metadata = m_framebufferMetadata[cmd.framebuffer];
		metadata.width = cmd.width;
		metadata.height = cmd.height;

		if (!metadata.isWindowSurface)
		{
			for (const auto& texHandle : metadata.colorAttachments)
			{
				if (texHandle.IsValid())
				{
					auto& texMetadata = m_textureMetadata[texHandle];
					CreateTextureStorageCmd storageCmd;
					storageCmd.texture = texHandle;
					storageCmd.width = cmd.width;
					storageCmd.height = cmd.height;
					storageCmd.format = texMetadata.format;
					storageCmd.usage = texMetadata.usage;
					ExecuteCreateTextureStorage(storageCmd);
				}
			}

			if (metadata.depthStencilAttachment.IsValid())
			{
				auto& texMetadata = m_textureMetadata[metadata.depthStencilAttachment];
				CreateTextureStorageCmd storageCmd;
				storageCmd.texture = metadata.depthStencilAttachment;
				storageCmd.width = cmd.width;
				storageCmd.height = cmd.height;
				storageCmd.format = texMetadata.format;
				storageCmd.usage = texMetadata.usage;
				ExecuteCreateTextureStorage(storageCmd);
			}

			BakeFramebufferCmd bakeCmd;
			bakeCmd.framebuffer = cmd.framebuffer;
			ExecuteBakeFramebuffer(bakeCmd);
		}
	}

	void ExecuteDestroyFramebuffer(const DestroyFramebufferCmd& cmd)
	{
		auto it = m_framebuffers.find(cmd.handle);
		if (it == m_framebuffers.end())
		{
			SableUI_Warn("Attempting to destroy non-existent framebuffer");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.handle];
		if (!metadata.isWindowSurface && it->second != 0)
		{
			GLuint fboID = it->second;
			glDeleteFramebuffers(1, &fboID);
		}

		m_framebuffers.erase(it);
		m_framebufferMetadata.erase(cmd.handle);
	}

	// ============================================================================
	// UBO
	// ============================================================================

	void ExecuteCreateUniformBuffer(const CreateUniformBufferCmd& cmd,
		const std::vector<uint8_t>& initialData)
	{
		GLuint uboID;
		glGenBuffers(1, &uboID);
		glBindBuffer(GL_UNIFORM_BUFFER, uboID);

		const void* data = initialData.empty() ? nullptr : initialData.data();
		glBufferData(GL_UNIFORM_BUFFER, cmd.size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		m_uniformBuffers[cmd.handle] = uboID;
	}

	void ExecuteUpdateUniformBuffer(const UpdateUniformBufferCmd& cmd,
		const std::vector<uint8_t>& data)
	{
		auto it = m_uniformBuffers.find(cmd.buffer);
		if (it == m_uniformBuffers.end())
		{
			SableUI_Error("Attempting to update non-existent uniform buffer");
			return;
		}

		glBindBuffer(GL_UNIFORM_BUFFER, it->second);
		glBufferSubData(GL_UNIFORM_BUFFER, cmd.offset, cmd.size, data.data());
	}

	void ExecuteDestroyUniformBuffer(const DestroyUniformBufferCmd& cmd)
	{
		auto it = m_uniformBuffers.find(cmd.handle);
		if (it == m_uniformBuffers.end())
		{
			SableUI_Warn("Attempting to destroy non-existent uniform buffer");
			return;
		}

		GLuint uboID = it->second;
		glDeleteBuffers(1, &uboID);
		m_uniformBuffers.erase(it);
	}

	// ============================================================================
	// Drawing
	// ============================================================================

	void ExecuteDrawGpuObject(const DrawGpuObjectCmd& cmd)
	{
		auto objIt = m_gpuObjects.find(cmd.handle);
		auto metaIt = m_meshMetadata.find(cmd.handle);

		if (objIt == m_gpuObjects.end() || metaIt == m_meshMetadata.end())
		{
			SableUI_Error("Attempting to draw non-existent GPU object");
			return;
		}

		const OpenGL3Backend::OpenGLMesh& mesh = objIt->second;
		const GpuObjectMetadata& metadata = metaIt->second;

		glBindVertexArray(mesh.vao);

		if (cmd.instanceCount > 1)
		{
			glDrawElementsInstancedBaseVertex(
				GL_TRIANGLES,
				metadata.indexCount,
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
				metadata.indexCount,
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
		{
			glDrawArraysInstanced(GL_TRIANGLES, cmd.firstVertex,
				cmd.vertexCount, cmd.instanceCount);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, cmd.firstVertex, cmd.vertexCount);
		}
	}

	// ============================================================================
	// Render pass
	// ============================================================================

	void ExecuteClear(const ClearCmd& cmd)
	{
		glClearColor(cmd.r, cmd.g, cmd.b, cmd.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void ExecuteBeginRenderPass(const BeginRenderPassCmd& cmd)
	{
		auto fboIt = m_framebuffers.find(cmd.framebuffer);
		if (fboIt == m_framebuffers.end())
		{
			SableUI_Error("Attempting to begin render pass with non-existent framebuffer");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.framebuffer];

		if (metadata.isWindowSurface)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboIt->second);

			if (!metadata.colorAttachments.empty() &&
				metadata.colorAttachments[0].IsValid())
			{
				auto& texMetadata = m_textureMetadata[metadata.colorAttachments[0]];
				glViewport(0, 0, texMetadata.width, texMetadata.height);
			}
		}
	}

	void ExecuteBlitFramebuffer(const BlitFramebufferCmd& cmd)
	{
		auto srcIt = m_framebuffers.find(cmd.srcFramebuffer);
		auto dstIt = m_framebuffers.find(cmd.dstFramebuffer);

		if (srcIt == m_framebuffers.end() || dstIt == m_framebuffers.end())
		{
			SableUI_Error("Invalid framebuffer handle in BlitFramebuffer");
			return;
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcIt->second);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstIt->second);

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
		auto srcIt = m_framebuffers.find(cmd.framebuffer);
		if (srcIt == m_framebuffers.end())
		{
			SableUI_Error("Invalid framebuffer handle in BlitToScreen");
			return;
		}

		auto& metadata = m_framebufferMetadata[cmd.framebuffer];

		glBindFramebuffer(GL_READ_FRAMEBUFFER, srcIt->second);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(
			0, 0, metadata.width, metadata.height,
			0, 0, metadata.width, metadata.height,
			GL_COLOR_BUFFER_BIT,
			TextureInterpolationToGL(cmd.filter)
		);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ExecuteCreateGpuObject(const CreateGpuObjectCmd& cmd,
		const std::vector<uint8_t>& data)
	{
		OpenGL3Backend::OpenGLMesh mesh;

		glGenVertexArrays(1, &mesh.vao);
		glBindVertexArray(mesh.vao);

		size_t vertexDataSize = static_cast<size_t>(cmd.numVertices) * cmd.layout.stride;
		const void* vertexData = data.data();
		const uint32_t* indexData = nullptr;

		if (cmd.numIndices > 0)
			indexData = reinterpret_cast<const uint32_t*>(data.data() + vertexDataSize);

		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);

		if (indexData && cmd.numIndices > 0)
		{
			glGenBuffers(1, &mesh.ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				cmd.numIndices * sizeof(uint32_t),
				indexData,
				GL_STATIC_DRAW);
		}

		uint32_t attrIndex = 0;
		for (const auto& attr : cmd.layout.attributes)
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
					cmd.layout.stride,
					reinterpret_cast<void*>(static_cast<uintptr_t>(attr.offset))
				);
			}
			else
			{
				glVertexAttribPointer(
					attrIndex,
					count,
					type,
					normalized,
					cmd.layout.stride,
					reinterpret_cast<void*>(static_cast<uintptr_t>(attr.offset))
				);
			}
			attrIndex++;
		}

		glBindVertexArray(0);

		m_gpuObjects[cmd.handle] = mesh;

		GpuObjectMetadata metadata;
		metadata.vertexCount = cmd.numVertices;
		metadata.indexCount = cmd.numIndices;
		m_meshMetadata[cmd.handle] = metadata;
	}

	void ExecuteDestroyGpuObject(const DestroyGpuObjectCmd& cmd)
	{
		auto it = m_gpuObjects.find(cmd.handle);
		if (it == m_gpuObjects.end())
		{
			SableUI_Warn("Attempting to destroy non-existent GPU object (handle index: %u)",
				cmd.handle.index);
			return;
		}

		OpenGL3Backend::OpenGLMesh& mesh = it->second;

		if (mesh.vao != 0)
			glDeleteVertexArrays(1, &mesh.vao);

		if (mesh.vbo != 0)
			glDeleteBuffers(1, &mesh.vbo);

		if (mesh.ebo != 0)
			glDeleteBuffers(1, &mesh.ebo);

		m_gpuObjects.erase(it);
		m_meshMetadata.erase(cmd.handle);
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