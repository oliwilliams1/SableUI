#pragma once
#include <SableUI/drawable.h>
#include <cstdint>
#include <vector>
#include <SableUI/utils.h>
#include "memory.h"

namespace SableUI
{
	// RenderTarget
	enum class RenderTargetType
	{
		Window,
		Texture
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

	// Object
	enum class VertexFormat : uint16_t
	{
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
		}
		return 0;
	}

	struct VertexAttribute
	{
		uint16_t offset;
		VertexFormat format;
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

	class RendererBackend;
	struct GpuObject
	{
		static int GetNumInstances();
		GpuObject();
		~GpuObject();
		RendererBackend* m_context = nullptr;
		uint32_t m_handle = 0;
		uint32_t numVertices = 0;
		uint32_t numIndices = 0;
		VertexLayout layout{};

		void AddToDrawStack() const;
	};

	// Renderer
	enum class Backend { UNDEF, OpenGL, Vulkan, DirectX, Metal };

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

	struct GpuFramebuffer;
	struct Element;
	struct CustomTargetQueue
	{
		CustomTargetQueue(const GpuFramebuffer* target) { this->target = target; }
		const GpuFramebuffer* target = nullptr;
		std::vector<DrawableBase*> drawables;
		Element* root = nullptr;

		void AddRect(Rect rect, Colour colour);
	};

	struct BlitCommand
	{
		GpuFramebuffer* source = nullptr;
		GpuFramebuffer* target = nullptr;
		Rect sourceRect = { 0, 0, 0, 0 };
		Rect destRect = { 0, 0, 0, 0 };
		TextureInterpolation interpolation = TextureInterpolation::Nearest;
	};

	class RendererBackend
	{
	public:
		static RendererBackend* Create(Backend backend);
		~RendererBackend() = default;
		virtual void Initalise() = 0;
		virtual void Clear(float r, float g, float b, float a) = 0;
		virtual void Viewport(int x, int y, int width, int height) = 0;
		virtual void SetBlending(bool enabled) = 0;
		virtual void SetBlendFunction(BlendFactor src, BlendFactor dst) = 0;
		virtual void Flush() = 0;
		virtual void CheckErrors() = 0;
		
		virtual void ClearDrawableStack() = 0;
		virtual void ClearDrawable(const DrawableBase* drawable) = 0;
		virtual void AddToDrawStack(DrawableBase* drawable) = 0;
		virtual void AddToDrawStack(const GpuObject* obj) = 0;
		virtual bool Draw(const GpuFramebuffer* target) = 0;

		virtual GpuObject* CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout) = 0;
		virtual void DestroyGpuObject(GpuObject* obj) = 0;

		virtual void BeginRenderPass(const GpuFramebuffer* fbo) = 0;
		virtual void EndRenderPass() = 0;

		virtual void BlitToScreen(GpuFramebuffer* source,
			TextureInterpolation interpolation = TextureInterpolation::Nearest) = 0;
		virtual void BlitToFramebuffer(
			GpuFramebuffer* source, GpuFramebuffer* target,
			Rect sourceRect, Rect destRect,
			TextureInterpolation interpolation = TextureInterpolation::Nearest) = 0;

		bool isDirty() const { return !m_drawStack.empty(); };

	protected:
		uint32_t AllocateHandle();
		void FreeHandle(uint32_t handle);
		uint32_t m_nextHandle = 0;
		std::vector<uint32_t> m_freeHandles;
		Backend m_backend = Backend::UNDEF;

		bool m_directDraw = false;
		std::vector<DrawableBase*> m_drawStack;
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

	struct GpuTexture2D
	{
	public:
		~GpuTexture2D();
		void Bind(uint32_t slot = 0) const;
		void Unbind(uint32_t slot = 0) const;

		void CreateStorage(int width, int height, TextureFormat format, TextureUsage usage);
		void SetData(const uint8_t* pixels, int width, int height, TextureFormat format);

		uint32_t GetHandle() const { return m_textureID; }
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		TextureFormat GetFormat() const { return m_format; }
		TextureUsage GetUsage() const { return m_usage; }

	private:
		int m_width = 0, m_height = 0;
		uint32_t m_textureID = 0;
		TextureFormat m_format = TextureFormat::Undefined;
		TextureUsage m_usage = TextureUsage::ShaderSample;
	};

	struct GpuTexture2DArray
	{
	public:
		GpuTexture2DArray() = default;
		~GpuTexture2DArray();
		void Bind() const;
		void Unbind() const;
		void Init(int width, int height, int depth);
		void Resize(int newDepth);
		void SubImage(int xOffset, int yOffset, int zOffset, int width, int height,
			int depth, const uint8_t* pixels);
		void CopyImageSubData(const GpuTexture2DArray& src, int srcX, int srcY, int srcZ,
			int dstX, int dstY, int dstZ, int width, int height, int depth);

		uint32_t m_textureID = 0;

		GpuTexture2DArray(GpuTexture2DArray&& other) noexcept;
		GpuTexture2DArray& operator=(GpuTexture2DArray&& other) noexcept;
		GpuTexture2DArray(const GpuTexture2DArray&) = delete;
		GpuTexture2DArray& operator=(const GpuTexture2DArray&) = delete;

	private:
		int m_width = 0, m_height = 0, m_depth = 0;
	};

	struct GpuFramebuffer
	{
	public:
		~GpuFramebuffer();

		void AttachColour(GpuTexture2D* texture, int slot = 0);
		void AttachDepthStencil(GpuTexture2D* texture);
		void Bake();
		void SetSize(int width, int height);
		uint32_t GetHandle() const { return m_handle; }
		void SetIsWindowSurface(bool b) { isWindowSurface = b; }

		const std::vector<GpuTexture2D>& GetColorAttachments() const { return m_colorAttachments; }
		const GpuTexture2D& GetDepthAttachment() const { return m_depthStencilAttachment; }

		int width = 0, height = 0;
		bool isWindowSurface = false;
	
	private:
		uint32_t m_handle = 0;

		std::vector<GpuTexture2D> m_colorAttachments;
		GpuTexture2D m_depthStencilAttachment;
	};

	inline uint32_t RendererBackend::AllocateHandle()
	{
		if (!m_freeHandles.empty())
		{
			uint32_t handle = m_freeHandles.back();
			m_freeHandles.pop_back();
			return handle;
		}
		return m_nextHandle++;
	}

	inline void RendererBackend::FreeHandle(uint32_t handle)
	{
		m_freeHandles.push_back(handle);
	}

	inline void CustomTargetQueue::AddRect(Rect rect, Colour colour)
	{
		DrawableRect* drRect = SableMemory::SB_new<DrawableRect>();
		drRect->m_rect = rect;
		drRect->m_colour = colour;
		drRect->orphan = true;
		drawables.push_back(drRect);
	}
}