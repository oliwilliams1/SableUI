#pragma once
#include <SableUI/core/component.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/utils/utils.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/states/state_base.h>
#include <SableUI/core/window.h>
#include <SableUI/types/floating_panel_types.h>
#include <SableUI/utils/memory.h>
#include <type_traits>

namespace SableUI
{
	template <typename T>
	class FloatingPanel : public FloatingPanelBase
	{
		static_assert(std::is_base_of_v<BaseComponent, T>,
			"FloatingPanel<T>: T must derive from BaseComponent");
	public:
		FloatingPanel(BaseComponent* owner);
		~FloatingPanel();

		void Open(Rect rect = {});
		void Open(int x, int y, int width, int height);
		void Close();
		void Toggle();
		bool IsOpen() const;

		void SetPosition(int x, int y);
		void SetSize(int width, int height);
		void SetRect(Rect rect);

		void Sync(StateBase* other) override;

	private:
		ResourceHandle m_framebuffer;
		ResourceHandle m_texture;
		Rect m_rect{};

		bool m_open = false;
		T* m_child = nullptr;
		BaseComponent* m_owner = nullptr;
		inline static int s_nextId = 0;
		int m_stableId = -1;
	};

	template<typename T>
	inline FloatingPanel<T>::FloatingPanel(BaseComponent* owner)
	{
		m_owner = owner;
		m_stableId = s_nextId++;

		m_owner->RegisterState(this);
		m_owner->RegisterFloatingPanel(this);
	}

	template<typename T>
	inline FloatingPanel<T>::~FloatingPanel()
	{
		if (m_stableId >= 0)
			if (Window* ctx = _getCurrentContext())
				ctx->UnregisterFloatingPanel(m_stableId);

		// if m_stableId == -1, this means Sync() transferred ownership, no need to unregister

		if (m_child)
			SableMemory::SB_delete(m_child);
	}

	template<typename T>
	inline void FloatingPanel<T>::Open(Rect rect)
	{
		m_rect = rect;
		m_open = true;

		Window* ctx = _getCurrentContext();
		FloatingPanelEntry& entry = ctx->GetFloatingPanelEntry(m_stableId);
		CommandBuffer& mainCB = ctx->GetMainCommandBuffer();

		bool needsGpuAlloc = !entry.texture.IsValid();
		bool needsGpuResize = entry.size.x != rect.w || entry.size.y != rect.h;

		if (needsGpuAlloc)
		{
			entry.texture = mainCB.CreateTexture2D(rect.w, rect.h, TextureFormat::RGBA8, TextureUsage::RenderTarget);
			entry.framebuffer = mainCB.CreateFramebuffer(rect.w, rect.h);
			mainCB.AttachColourTexture(entry.framebuffer, entry.texture, 0);
			mainCB.BakeFramebuffer(entry.framebuffer);
			entry.cb = ctx->GetRenderer()->CreateSecondaryCommandBuffer();

			m_child->SetRenderer(ctx->GetRenderer());
			m_child->BackendInitialiseFloatingPanel(rect);
		}
		else if (needsGpuResize)
		{
			mainCB.SetFramebufferSize(entry.framebuffer, rect.w, rect.h);
			m_child->GetRootElement()->SetRect(rect);
		}

		entry.pos = { rect.x, rect.y };
		entry.size = { rect.w, rect.h };
		entry.dirty = true;
		ctx->RebuildCompositeCommandBuffer();
		PostEmptyEvent();
	}

	template<typename T>
	inline void FloatingPanel<T>::Open(int x, int y, int width, int height)
	{
		Open({ x, y, width, height });
	}

	template<typename T>
	void FloatingPanel<T>::Close()
	{
		if (!m_open) return;
		m_open = false;

		Window* ctx = _getCurrentContext();
		FloatingPanelEntry& entry = ctx->GetFloatingPanelEntry(m_stableId);
		CommandBuffer& mainCB = ctx->GetMainCommandBuffer();

		if (entry.texture.IsValid())
		{
			mainCB.DestroyTexture(entry.texture);
			entry.texture = {};
		}
		if (entry.framebuffer.IsValid())
		{
			mainCB.DestroyFramebuffer(entry.framebuffer);
			entry.framebuffer = {};
		}
		entry.cb = {};
		entry.size = {};

		ctx->RebuildCompositeCommandBuffer();
		PostEmptyEvent();
	}

	template<typename T>
	inline void FloatingPanel<T>::Toggle()
	{
		m_open = !m_open;
	}

	template<typename T>
	inline bool FloatingPanel<T>::IsOpen() const
	{
		return m_open;
	}

	template<typename T>
	inline void FloatingPanel<T>::SetPosition(int x, int y)
	{
		m_rect.x = x;
		m_rect.y = y;
	}

	template<typename T>
	inline void FloatingPanel<T>::SetSize(int width, int height)
	{
		m_rect.w = width;
		m_rect.h = height;
	}

	template<typename T>
	inline void FloatingPanel<T>::SetRect(Rect rect)
	{
		m_rect = rect;
	}

	template<typename T>
	inline void FloatingPanel<T>::Sync(StateBase* other)
	{
		auto* otherPtr = static_cast<FloatingPanel<T>*>(other);
		m_open = otherPtr->m_open;
		m_rect = otherPtr->m_rect;

		if (m_child && otherPtr->m_child)
			m_child->CopyStateFrom(*otherPtr->m_child);

		_getCurrentContext()->ReassociateFloatingPanel(m_stableId, this);
		otherPtr->m_stableId = -1;
	}
}