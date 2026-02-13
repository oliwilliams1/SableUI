#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/core/shader.h>
#include <SableUI/renderer/gpu_object.h>
#include <SableUI/renderer/gpu_framebuffer.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/renderer/resource_handle.h>
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

	// SableUI-specific resources
	struct GlobalResources {
		bool initialised = false;
		Shader s_rect;
		uint32_t ubo_rect = static_cast<uint32_t>(-1);

		Shader s_text;
		uint32_t ubo_text = static_cast<uint32_t>(-1);
		uint32_t u_textAtlas = static_cast<uint32_t>(-1);
	};

	struct ContextResources {
		ResourceHandle rectObject;
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

		virtual uint32_t CreateUniformBuffer(size_t size, const void* initialData = nullptr) = 0;
		virtual void DestroyUniformBuffer(uint32_t ubo) = 0;
		virtual void BindUniformBufferBase(uint32_t binding, uint32_t ubo) = 0;

		CommandBuffer& GetCommandBuffer() { return m_commandBuffer; };
		void ResetCommandBuffer() { m_commandBuffer.Reset(); };
		virtual void ExecuteCommandBuffer() = 0;

		CommandBuffer CreateSecondaryCommandBuffer() { return CommandBuffer(&m_resourceAllocator); }
		
		virtual GpuObject* CreateGpuObject(
			const void* vertices, uint32_t numVertices,
			const uint32_t* indices, uint32_t numIndices,
			const VertexLayout& layout) = 0;
		virtual void DestroyGpuObject(GpuObject* obj) = 0;

		virtual GpuObjectMetadata& GetMeshMetadata(uint32_t handle) = 0;

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
		ResourceHandleAllocator& GetResourceAllocator() { return m_resourceAllocator; }

	protected:
		uint32_t AllocateHandle();
		void FreeHandle(uint32_t handle);
		uint32_t m_nextHandle = 0;
		std::vector<uint32_t> m_freeHandles;
		Backend m_backend = Backend::Undef;

		CommandBuffer m_commandBuffer;
		CommandBufferExecutor* m_executor = nullptr;
		ResourceHandleAllocator m_resourceAllocator;
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