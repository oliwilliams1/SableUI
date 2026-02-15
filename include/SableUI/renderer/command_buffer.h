#pragma once
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/resource_handle.h>
#include <cstdint>
#include <vector>
#include <optional>

namespace SableUI
{
	// Forward declarations
	struct GlobalResources;
	struct ContextResources;
	class RendererBackend;

	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		explicit CommandBuffer(ResourceHandleAllocator* allocator) : m_allocator(allocator) {};
		~CommandBuffer() = default;

		void Reset();
		void SetAllocator(ResourceHandleAllocator* allocator) { m_allocator = allocator; }

		void SetPipeline(PipelineType pipeline);
		void SetBlendState(bool enabled = true,
			BlendFactor src = BlendFactor::SrcAlpha,
			BlendFactor dst = BlendFactor::OneMinusSrcAlpha);
		void SetScissor(int x, int y, int width, int height);
		void DisableScissor();
		void SetViewport(int x, int y, int width, int height);

		void BindUniformBuffer(uint32_t binding, ResourceHandle buffer);
		void BindTexture(uint32_t slot, ResourceHandle texture);
		void BindGpuObject(ResourceHandle handle);
		void BindFramebuffer(ResourceHandle framebuffer);

		ResourceHandle CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout);
		void DestroyGpuObject(ResourceHandle handle);

		ResourceHandle CreateTexture2D(
			int width, int height,
			TextureFormat format,
			TextureUsage usage = TextureUsage::ShaderSample);

		void SetDataTexture2D(ResourceHandle texture,
			const uint8_t* pixels, int width, int height,
			TextureFormat format);

		void CreateStorageTexture2D(ResourceHandle texture,
			int width, int height,
			TextureFormat format,
			TextureUsage usage);
		
		void DestroyTexture2D(ResourceHandle handle);

		ResourceHandle CreateTexture2DArray(
			int width, int height, int layers,
			TextureFormat format,
			TextureUsage usage = TextureUsage::ShaderSample);

		void SubImageTexture2DArray(ResourceHandle texture,
			int xOffset, int yOffset, int zOffset,
			int width, int height, int depth,
			TextureFormat format,
			const uint8_t* pixels);

		void CopyImageDataTexture2DArray(ResourceHandle src, ResourceHandle dst,
			int srcX, int srcY, int srcZ,
			int dstX, int dstY, int dstZ,
			int width, int height, int depth);

		void ResizeTexture2DArray(ResourceHandle handle, int newDepth);

		ResourceHandle CreateFramebuffer(int width = 0, int height = 0, bool isWindowSurface = false);
		void AttachColourTexture(ResourceHandle framebuffer, ResourceHandle texture, int slot = 0);
		void AttachDepthStencilTexture(ResourceHandle framebuffer, ResourceHandle texture);
		void BakeFramebuffer(ResourceHandle framebuffer);
		void SetFramebufferSize(ResourceHandle framebuffer, int width, int height);
		void DestroyFramebuffer(ResourceHandle handle);

		ResourceHandle CreateUniformBuffer(uint32_t size, const void* initialData = nullptr);
		void UpdateUniformBuffer(ResourceHandle buffer, uint32_t offset, uint32_t size, const void* data);
		void DestroyUniformBuffer(ResourceHandle handle);

		void DrawGpuObject(ResourceHandle object, uint32_t instanceCount = 1);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
			uint32_t firstIndex = 0, int32_t vertexOffset = 0,
			uint32_t firstInstance = 0);
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
			uint32_t firstVertex = 0, uint32_t firstInstance = 0);

		void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
		void BeginRenderPass(ResourceHandle framebuffer);
		void EndRenderPass();

		void BlitFramebuffer(ResourceHandle srcFramebuffer, ResourceHandle dstFramebuffer,
			int srcX0, int srcY0, int srcX1, int srcY1,
			int dstX0, int dstY0, int dstX1, int dstY1,
			TextureInterpolation filter = TextureInterpolation::Nearest);

		void BlitToScreen(ResourceHandle framebuffer,
			TextureInterpolation filter = TextureInterpolation::Nearest);

		const std::vector<Command>& GetCommands() const { return m_commands; }
		bool empty() const { return m_commands.empty(); }
		size_t GetCommandCount() const { return m_commands.size(); }

	private:
		ResourceHandleAllocator* m_allocator = nullptr;
		std::vector<Command> m_commands;

		struct State
		{
			std::optional<PipelineType> pipeline;
		} m_state;
	};

	class CommandBufferExecutor
	{
	public:
		static CommandBufferExecutor* Create(
			Backend backend,
			GlobalResources* globalRes,
			ContextResources* contextRes,
			RendererBackend* renderer
		);
		virtual ~CommandBufferExecutor() = default;
		virtual void Execute(const CommandBuffer& cmdBuffer) = 0;
	};
}