#pragma once
#include <SableUI/renderer/gpu_texture.h>
#include <cstdint>
#include <vector>

namespace SableUI
{
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
}