#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/core/shader.h>
#include <SableUI/renderer/gpu_texture.h>
#include <SableUI/renderer/gpu_object.h>
#include <SableUI/renderer/gpu_framebuffer.h>
#include <SableUI/types/renderer_types.h>
#include <cstdint>
#include <vector>
#include <optional>

namespace SableUI
{
	// Forward delcarations
	struct Element;
	class Window;
	class DrawableBase;
	class CommandBuffer;
	class RendererBackend;

	// SableUI-specific resources
	struct GlobalResources {
		Shader s_rect;
		uint32_t ubo_rect = 0;

		Shader s_text;
		uint32_t ubo_text = 0;
		uint32_t u_textAtlas = 0;
	};

	struct ContextResources {
		GpuObject* rectObject = nullptr;
	};

	// ============================================================================
	// Command Buffers
	// ============================================================================
	class CommandBuffer
	{
	public:
		CommandBuffer() = default;
		~CommandBuffer() = default;

		void Reset();

		void SetPipeline(PipelineType pipeline);
		void SetBlendState(bool enabled, BlendFactor src, BlendFactor dst);
		void SetScissor(int x, int y, int width, int height);
		void DisableScissor();

		void BindGpuObject(uint32_t handle);
		void BindUniformBuffer(uint32_t binding, uint32_t ubo);
		void BindTexture(uint32_t slot, const GpuTexture* texture);

		void UpdateUniformBuffer(uint32_t ubo, uint32_t offset, uint32_t size, const void* data);

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

		const std::vector<Command>& GetCommands() const { return m_commands; }
		bool empty() const { return m_commands.empty(); }
		size_t GetCommandCount() const { return m_commands.size(); }

	private:
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

	// ============================================================================
	// Renderer
	// ============================================================================
	class RendererBackend
	{
	public:
		static RendererBackend* Create(Backend backend);
		virtual ~RendererBackend() = default;
		virtual void Initialise() = 0;
		virtual void Clear(float r, float g, float b, float a) = 0;
		virtual void Viewport(int x, int y, int width, int height) = 0;
		virtual void SetBlending(bool enabled) = 0;
		virtual void SetBlendFunction(BlendFactor src, BlendFactor dst) = 0;
		virtual void CheckErrors() = 0;

		CommandBuffer& GetCommandBuffer() { return m_commandBuffer; };
		void ResetCommandBuffer() { m_commandBuffer.Reset(); };
		virtual void ExecuteCommandBuffer() = 0;
		
		virtual GpuObject* CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout) = 0;
		virtual void DestroyGpuObject(GpuObject* obj) = 0;

		virtual void BeginRenderPass(const GpuFramebuffer* fbo) = 0;
		virtual void EndRenderPass() = 0;

		virtual void BlitToScreen(GpuFramebuffer* source,
			TextureInterpolation interpolation = TextureInterpolation::Nearest) = 0;
		virtual void BlitToScreenWithRects(
			GpuFramebuffer* source,
			const Rect& sourceRect,
			const Rect& destRect,
			TextureInterpolation interpolation = TextureInterpolation::Nearest) = 0;
		virtual void BlitToFramebuffer(
			GpuFramebuffer* source, GpuFramebuffer* target,
			Rect sourceRect, Rect destRect,
			TextureInterpolation interpolation = TextureInterpolation::Nearest) = 0;
		virtual void DrawToScreen(
			GpuFramebuffer* source,
			const Rect& sourceRect,
			const Rect& destRect,
			const ivec2& windowSize) = 0;

		bool isDirty() const { return !m_commandBuffer.empty(); };

	protected:
		uint32_t AllocateHandle();
		void FreeHandle(uint32_t handle);
		uint32_t m_nextHandle = 0;
		std::vector<uint32_t> m_freeHandles;
		Backend m_backend = Backend::Undef;

		CommandBuffer m_commandBuffer;
		CommandBufferExecutor* m_executor = nullptr;
	};

	struct CustomTargetQueue
	{
	public:
		CustomTargetQueue();
		~CustomTargetQueue();
		static int GetNumInstances();
		Window* window = nullptr;
		GpuFramebuffer* target = nullptr;
		std::vector<DrawableBase*> drawables;

		void AddRect(
			const Rect& rect,
			const Colour& colour,
			float rTL = 0.0f, float rTR = 0.0f,
			float rBL = 0.0f, float rBR = 0.0f,
			std::optional<Colour> borderColour = std::nullopt,
			int bT = 0, int bB = 0,
			int bL = 0, int bR = 0);

		CustomTargetQueue(const CustomTargetQueue&) = delete;
		CustomTargetQueue& operator=(const CustomTargetQueue&) = delete;
		CustomTargetQueue(CustomTargetQueue&& other) = delete;
		CustomTargetQueue& operator=(CustomTargetQueue&& other) = delete;
	};
}