#pragma once
#include <SableUI/renderer/resource_handle.h>
#include <cstdint>
#include <vector>
#include <variant>

namespace SableUI
{
	enum class Backend { Undef, OpenGL, Vulkan, DirectX, Metal };

	struct FramebufferMetadata
	{
		int width = 0;
		int height = 0;
		bool isWindowSurface = false;
		std::vector<ResourceHandle> colorAttachments;
		ResourceHandle depthStencilAttachment;
	};

	struct TextureMetadata
	{
		int width = 0;
		int height = 0;
		int depth = -1;
		TextureFormat format = TextureFormat::Undefined;
		TextureUsage usage = TextureUsage::ShaderSample;
		TextureType type = TextureType::Texture2D;
	};

	struct GpuObjectMetadata
	{
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
	};

	enum class CommandType : uint8_t
	{
		SetPipeline,
		SetBlendState,
		SetScissor,
		DisableScissor,
		Clear,

		BlitFramebuffer,
		BlitToScreen,
		SetViewport,
		
		CreateGpuObject,
		BindGpuObject,
		DrawGpuObject,
		DestroyGpuObject,

		CreateTexture2D,
		CreateStorageTexture2D,
		BindTexture,
		DestroyTexture2D,
		SetDataTexture2D,
		
		CreateTexture2DArray,
		ResizeTexture2DArray,
		SubImageTexture2DArray,
		CopyImageDataTexture2DArray,
		
		CreateUniformBuffer,
		BindUniformBuffer,
		UpdateUniformBuffer,
		DestroyUniformBuffer,

		CreateFramebuffer,
		DestroyFramebuffer,
		BindFramebuffer,
		AttachColourTexture,
		AttachDepthStencilTexture,
		BakeFramebuffer,
		SetFramebufferSize,
		
		DrawIndexed,
		Draw,

		BeginRenderPass,
		EndRenderPass		
	};

	enum class PipelineType : uint8_t
	{
		Rect,
		Text,
		Image,
	};

	enum class TextureType
	{
		Texture2D,
		Texture2DArray
	};

	enum class RenderTargetType
	{
		Window,
		Texture
	};

	enum class VertexFormat : uint16_t
	{
		Undef,
		Float1,
		Float2,
		Float3,
		Float4,
		UInt1,
		UInt2,
		UInt3,
		UInt4,
		Int1,
		Int2,
		Int3,
		Int4
	};

	constexpr uint16_t GetFormatSize(VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::Float1: return 4;
		case VertexFormat::Float2: return 8;
		case VertexFormat::Float3: return 12;
		case VertexFormat::Float4: return 16;
		case VertexFormat::UInt1: return 4;
		case VertexFormat::UInt2: return 8;
		case VertexFormat::UInt3: return 12;
		case VertexFormat::UInt4: return 16;
		case VertexFormat::Int1: return 4;
		case VertexFormat::Int2: return 8;
		case VertexFormat::Int3: return 12;
		case VertexFormat::Int4: return 16;
		default: return 0;
		}
		return 0;
	}

	struct VertexAttribute
	{
		uint16_t offset = 0;
		VertexFormat format = VertexFormat::Undef;
		bool normalised = false;
	};

	struct VertexLayout
	{
		std::vector<VertexAttribute> attributes;
		uint16_t stride = 0;
		uint16_t currentOffset = 0;

		void Add(VertexFormat format)
		{
			VertexAttribute attr;
			attr.format = format;
			attr.offset = currentOffset;
			attributes.push_back(attr);
			currentOffset += GetFormatSize(format);
			stride = currentOffset;
		}
	};

	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate
	};

	enum class TextureInterpolation
	{
		Nearest,
		Linear
	};

	enum class TextureFormat
	{
		RGBA8,
		RGB8,
		RG8,
		R8,
		Undefined
	};

	enum class TextureUsage
	{
		ShaderSample,
		RenderTarget,
		Storage
	};

	// ============================================================================
	// Command Structures
	// ============================================================================
	struct SetPipelineCmd
	{
		PipelineType pipeline;
	};

	struct SetBlendStateCmd
	{
		bool enabled = true;
		BlendFactor srcFactor = BlendFactor::SrcAlpha;
		BlendFactor dstFactor = BlendFactor::OneMinusSrcAlpha;
	};

	struct SetScissorCmd
	{
		int x = 0, y = 0, width = 0, height = 0;
	};

	struct SetViewportCmd
	{
		int x = 0, y = 0, width = 0, height = 0;
	};

	struct BindUniformBufferCmd
	{
		uint32_t binding;
		ResourceHandle buffer;
	};

	struct BindTextureCmd
	{
		uint32_t slot;
		ResourceHandle texture;
	};

	struct BindGpuObjectCmd
	{
		ResourceHandle handle;
	};

	struct BindFramebufferCmd
	{
		ResourceHandle framebuffer;
	};

	struct CreateGpuObjectCmd
	{
		ResourceHandle handle;
		uint32_t numVertices;
		uint32_t numIndices;
		VertexLayout layout;
	};

	struct DestroyGpuObjectCmd
	{
		ResourceHandle handle;
	};

	struct CreateTexture2DCmd
	{
		ResourceHandle handle;
		int width = 0;
		int height = 0;
		TextureFormat format = TextureFormat::RGBA8;
		TextureUsage usage = TextureUsage::ShaderSample;
	};

	struct DestroyTexture2DCmd
	{
		ResourceHandle handle;
	};

	struct SetTextureDataCmd
	{
		ResourceHandle texture;
		int width;
		int height;
		TextureFormat format;
	};

	struct CreateTextureStorageCmd
	{
		ResourceHandle texture;
		int width;
		int height;
		TextureFormat format;
		TextureUsage usage;
	};

	struct CreateTexture2DArrayCmd
	{
		ResourceHandle handle;
		int width = 0;
		int height = 0;
		int depth = 0;
		TextureFormat format = TextureFormat::RGBA8;
		TextureUsage usage = TextureUsage::ShaderSample;
	};

	struct ResizeTexture2DArrayCmd
	{
		ResourceHandle handle;
		int newDepth = 0;
	};

	struct SubImageTexture2DArrayCmd
	{
		ResourceHandle handle;
		int xOffset = 0;
		int yOffset = 0;
		int zOffset = 0;
		int width = 0;
		int height = 0;
		int depth = 0;
		TextureFormat format = TextureFormat::RGB8;
	};

	struct CopyImageDataTexture2DArrayCmd
	{
		ResourceHandle src;
		ResourceHandle dst;
		int srcX = 0;
		int srcY = 0;
		int srcZ = 0;
		int dstX = 0;
		int dstY = 0;
		int dstZ = 0;
		int width = 0;
		int height = 0;
		int depth = 0;
	};

	struct CreateFramebufferCmd
	{
		ResourceHandle handle;
		int width = 0;
		int height = 0;
		bool isWindowSurface = false;
	};

	struct DestroyFramebufferCmd
	{
		ResourceHandle handle;
	};

	struct AttachColorTextureCmd
	{
		ResourceHandle framebuffer;
		ResourceHandle texture;
		int slot = 0;
	};

	struct AttachDepthStencilTextureCmd
	{
		ResourceHandle framebuffer;
		ResourceHandle texture;
	};

	struct BakeFramebufferCmd
	{
		ResourceHandle framebuffer;
	};

	struct SetFramebufferSizeCmd
	{
		ResourceHandle framebuffer;
		int width;
		int height;
	};

	struct CreateUniformBufferCmd
	{
		ResourceHandle handle;
		uint32_t size;
	};

	struct DestroyUniformBufferCmd
	{
		ResourceHandle handle;
	};

	struct UpdateUniformBufferCmd
	{
		ResourceHandle buffer;
		uint32_t offset;
		uint32_t size;
	};

	struct DrawGpuObjectCmd
	{
		ResourceHandle handle;
		uint32_t instanceCount = 1;
	};

	struct DrawIndexedCmd
	{
		uint32_t indexCount;
		uint32_t instanceCount = 1;
		uint32_t firstIndex = 0;
		int32_t vertexOffset = 0;
		uint32_t firstInstance = 0;
	};

	struct DrawCmd
	{
		uint32_t vertexCount;
		uint32_t instanceCount = 1;
		uint32_t firstVertex = 0;
		uint32_t firstInstance = 0;
	};

	struct ClearCmd
	{
		float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
	};

	struct BeginRenderPassCmd
	{
		ResourceHandle framebuffer;
	};

	struct BlitFramebufferCmd
	{
		ResourceHandle srcFramebuffer;
		ResourceHandle dstFramebuffer;
		int srcX0 = 0, srcY0 = 0, srcX1 = 0, srcY1 = 0;
		int dstX0 = 0, dstY0 = 0, dstX1 = 0, dstY1 = 0;
		TextureInterpolation filter = TextureInterpolation::Nearest;
	};

	struct BlitToScreenCmd
	{
		ResourceHandle framebuffer;
		TextureInterpolation filter = TextureInterpolation::Nearest;
	};

	using CommandData = std::variant <
		SetPipelineCmd,
		SetBlendStateCmd,
		SetScissorCmd,
		SetViewportCmd,
		BindUniformBufferCmd,
		BindTextureCmd,
		BindGpuObjectCmd,
		BindFramebufferCmd,
		CreateGpuObjectCmd,
		DestroyGpuObjectCmd,
		CreateTexture2DCmd,
		DestroyTexture2DCmd,
		SetTextureDataCmd,
		CreateTextureStorageCmd,
		CreateTexture2DArrayCmd,
		ResizeTexture2DArrayCmd,
		SubImageTexture2DArrayCmd,
		CopyImageDataTexture2DArrayCmd,
		CreateFramebufferCmd,
		DestroyFramebufferCmd,
		AttachColorTextureCmd,
		AttachDepthStencilTextureCmd,
		BakeFramebufferCmd,
		SetFramebufferSizeCmd,
		CreateUniformBufferCmd,
		DestroyUniformBufferCmd,
		UpdateUniformBufferCmd,
		DrawGpuObjectCmd,
		DrawIndexedCmd,
		DrawCmd,
		ClearCmd,
		BeginRenderPassCmd,
		BlitFramebufferCmd,
		BlitToScreenCmd
	>;

	struct Command
	{
		CommandType type;
		CommandData data;
		std::vector<uint8_t> inlineData;
	};
}