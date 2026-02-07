#pragma once
#include <SableUI/types/renderer_types.h>
#include <cstdint>

namespace SableUI
{
	struct GpuTexture
	{
		virtual ~GpuTexture() = default;
		uint32_t GetHandle() const { return handle; }
		TextureType GetType() const { return type; };

	protected:
		uint32_t handle = 0;
		TextureType type = TextureType::Texture2D;
	};

	struct GpuTexture2D : public GpuTexture
	{
	public:
		GpuTexture2D() { type = TextureType::Texture2D; }
		~GpuTexture2D();
		void Bind(uint32_t slot = 0) const;
		void Unbind(uint32_t slot = 0) const;

		void CreateStorage(int width, int height, TextureFormat format, TextureUsage usage);
		void SetData(const uint8_t* pixels, int width, int height, TextureFormat format);

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		TextureFormat GetFormat() const { return m_format; }
		TextureUsage GetUsage() const { return m_usage; }

	private:
		int m_width = 0, m_height = 0;
		TextureFormat m_format = TextureFormat::Undefined;
		TextureUsage m_usage = TextureUsage::ShaderSample;
	};

	struct GpuTexture2DArray : public GpuTexture
	{
	public:
		GpuTexture2DArray() { type = TextureType::Texture2DArray; }
		~GpuTexture2DArray();
		void Bind(uint32_t slot = 0) const;
		void Unbind(uint32_t slot = 0) const;
		void Init(int width, int height, int depth);
		void Resize(int newDepth);
		void SubImage(int xOffset, int yOffset, int zOffset, int width, int height,
			int depth, const uint8_t* pixels);
		void CopyImageSubData(const GpuTexture2DArray& src, int srcX, int srcY, int srcZ,
			int dstX, int dstY, int dstZ, int width, int height, int depth);

	private:
		int m_width = 0, m_height = 0, m_depth = 0;
	};
}