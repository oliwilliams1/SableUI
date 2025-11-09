#pragma once
#include "SableUI/drawable.h"
#include <algorithm>

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
		UInt8_4,
		UInt16_2,
		UInt16_4
	};

	constexpr uint32_t FormatSize(VertexFormat format)
	{
		switch (format)
		{
		case VertexFormat::Float1: return 4;
		case VertexFormat::Float2: return 8;
		case VertexFormat::Float3: return 12;
		case VertexFormat::Float4: return 16;
		case VertexFormat::UInt8_4: return 4;
		case VertexFormat::UInt16_2: return 4;
		case VertexFormat::UInt16_4: return 8;
		}
		return 0;
	}

	struct VertexAttribute
	{
		uint16_t offset;
		VertexFormat format;
	};

	struct VertexLayout
	{
		std::vector<VertexAttribute> attributes;
		uint32_t stride = 0;

		void Add(uint16_t offset, VertexFormat format)
		{
			attributes.push_back({ offset, format });
			stride = std::max(stride, offset + FormatSize(format));
		}
	};

	class RendererBackend;
	struct GpuObject
	{
		static int GetNumInstances();
		~GpuObject();
		RendererBackend* m_context = nullptr;
		uint32_t m_handle;
		uint32_t numVertices;
		uint32_t numIndices;
		VertexLayout layout;

		void Draw() const;
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
		virtual void Draw(DrawableBase* drawable) = 0;
		virtual void Draw() = 0;
		virtual void Draw(const GpuObject* obj) = 0;
		virtual void StartDirectDraw() = 0;
		virtual void DirectDrawRect(const Rect& rect, const Colour& color) = 0;
		virtual void EndDirectDraw() = 0;

		virtual GpuObject* CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout) = 0;
		virtual void DestroyGpuObject(GpuObject* obj) = 0;

		RenderTarget m_renderTarget;
	
	protected:
		uint32_t AllocateHandle();
		void FreeHandle(uint32_t handle);
		uint32_t m_nextHandle = 0;
		std::vector<uint32_t> m_freeHandles;
		Backend m_backend = Backend::UNDEF;

		bool m_directDraw = false;
		std::vector<DrawableBase*> m_drawStack;
	};

	struct GpuTexture2D
	{
	public:
		~GpuTexture2D();
		void Bind() const;
		void Unbind() const;
		void SetData(const uint8_t* pixels, int width, int height, int channels);

	private:
		int m_width = 0, m_height = 0;
		uint32_t m_textureID = 0;
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
}