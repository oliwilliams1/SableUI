#pragma once
#include <SableUI/core/shader.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/core/text_cache.h>
#include <cstdint>
#include <vector>

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
		ResourceHandle ubo_rect;

		Shader s_text;
		ResourceHandle ubo_text;
		ResourceHandle u_textAtlas;
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
		virtual void CheckErrors() = 0;

		CommandBuffer& GetCommandBuffer() { return m_commandBuffer; };
		void ResetCommandBuffer() { m_commandBuffer.Reset(); };
		virtual void ExecuteCommandBuffer() = 0;

		CommandBuffer CreateSecondaryCommandBuffer() { return CommandBuffer(&m_resourceAllocator); }

		bool isDirty() const { return !m_commandBuffer.empty(); };
		ResourceHandleAllocator& GetResourceAllocator() { return m_resourceAllocator; }

		TextCacheFactory m_textCacheFactory;
	
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
}