#pragma once
#include <SableUI/renderer/resource_handle.h>
#include <cstdint>
#include <vector>
#include <variant>

namespace SableUI
{
	struct GpuFramebuffer;
	enum class Backend { Undef, OpenGL, Vulkan, DirectX, Metal };

	enum class CommandType : uint8_t
	{
		SetPipeline,
		SetBlendState,
		SetScissor,
		DisableScissor,
		BindUniformBuffer,
		BindTexture,
		CreateGpuObject,
		BindGpuObject,
		UpdateUniformBuffer,
		DestroyGpuObject,
		DeleteTexture2D,
		DeleteUniformBuffer,
		DrawIndexed,
		Draw,
		Clear,
		BeginRenderPass,
		EndRenderPass,
		BlitFramebuffer,
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
		uint16_t offset;
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

	struct SetPipelineCmd
	{
		PipelineType pipeline;
	};

	struct SetBlendStateCmd
	{
		bool enabled;
		BlendFactor srcFactor;
		BlendFactor dstFactor;
	};

	struct SetScissorCmd
	{
		int x, y, width, height;
	};

	struct BindUniformBufferCmd
	{
		uint32_t binding;
		uint32_t ubo;
	};

	struct BindTextureCmd
	{
		uint32_t slot;
		uint32_t handle;
		TextureType type;
	};

	struct CreateGpuObjectCmd
	{
		ResourceHandle handle;
		uint32_t numVertices;
		uint32_t numIndices;
		VertexLayout layout;
	};

	struct BindGpuObjectCmd
	{
		ResourceHandle handle;
	};

	struct DestroyGpuObjectCmd
	{
		ResourceHandle handle;
	};

	struct UpdateUniformBufferCmd
	{
		uint32_t ubo;
		uint32_t offset;
		uint32_t size;
	};

	struct DrawIndexedCmd
	{
		uint32_t indexCount;
		uint32_t instanceCount;
		uint32_t firstIndex;
		int32_t vertexOffset;
		uint32_t firstInstance;
	};

	struct DrawCmd
	{
		uint32_t vertexCount;
		uint32_t instanceCount;
		uint32_t firstVertex;
		uint32_t firstInstance;
	};

	struct ClearCmd
	{
		float r, g, b, a;
	};

	struct BeginRenderPassCmd
	{
		const GpuFramebuffer* framebuffer;
	};

	struct BlitFramebufferCmd
	{
		uint32_t srcFBO;
		uint32_t dstFBO;
		int srcX0, srcY0, srcX1, srcY1;
		int dstX0, dstY0, dstX1, dstY1;
		TextureInterpolation filter;
	};

	using CommandData = std::variant<
		SetPipelineCmd,
		SetBlendStateCmd,
		SetScissorCmd,
		BindGpuObjectCmd,
		BindUniformBufferCmd,
		BindTextureCmd,
		CreateGpuObjectCmd,
		DestroyGpuObjectCmd,
		UpdateUniformBufferCmd,
		DrawIndexedCmd,
		DrawCmd,
		ClearCmd,
		BeginRenderPassCmd,
		BlitFramebufferCmd
	>;

	struct Command
	{
		CommandType type;
		CommandData data;
		std::vector<uint8_t> inlineData;
	};

	struct RenderTarget
	{
		RenderTarget() = default;
		RenderTarget(int width, int height);
		~RenderTarget();

		void SetTarget(RenderTargetType target) { this->targetType = target; };
		void InitTexture();
		void Resize(int width, int height);

		void Bind() const;

		int width = 0, height = 0;
		RenderTargetType targetType = RenderTargetType::Window;

	private:
		uint32_t m_textureID = 0;
		void Update() const;
	};
}