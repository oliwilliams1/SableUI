#pragma once
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/gpu_texture.h>
#include <SableUI/renderer/gpu_framebuffer.h>
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
		void SetBlendState(bool enabled, BlendFactor src, BlendFactor dst);
		void SetScissor(int x, int y, int width, int height);
		void DisableScissor();

		void BindUniformBuffer(uint32_t binding, uint32_t ubo);
		void BindTexture(uint32_t slot, const GpuTexture* texture);

		ResourceHandle CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout);

		void BindGpuObject(ResourceHandle handle);
		void DestroyGpuObject(ResourceHandle handle);

		void UpdateUniformBuffer(uint32_t ubo, uint32_t offset, uint32_t size, const void* data);

		void DrawGpuObject(ResourceHandle object, uint32_t instanceCount = 1);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
			uint32_t firstIndex = 0, int32_t vertexOffset = 0,
			uint32_t firstInstance = 0);
		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
			uint32_t firstVertex = 0, uint32_t firstInstance = 0);

		void Clear(float r, float g, float b, float a);

		void BeginRenderPass(const GpuFramebuffer* framebuffer);
		void EndRenderPass();

		void BlitFramebuffer(uint32_t srcFBO, uint32_t dstFBO,
			int srcX0, int srcY0, int srcX1, int srcY1,
			int dstX0, int dstY0, int dstX1, int dstY1,
			TextureInterpolation filter);
		void BlitToScreen(const GpuFramebuffer* framebuffer);

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