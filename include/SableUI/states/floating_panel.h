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
#include <SableUI/core/events.h>
#include <SableUI/core/drawable.h>
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

		void Open(Rect rect);
		void Open(int x, int y, int width, int height);
		void Close();
		void Toggle(Rect rect);
		bool IsOpen() const override;
		int GetZIndex() const override;

		void Sync(StateBase* other) override;

		void PostLayoutUpdate(const UIEventContext& ctx) override;
		bool CheckAndUpdate(const DrawableDrawData& externalDrawData) override;
		void HandleInput(const UIEventContext& ctx, int z) override;

	private:
		ResourceHandle m_framebuffer;
		ResourceHandle m_texture;
		Rect m_rect{};

		bool m_open = false;
		T* m_child = nullptr;
		int zIndex = 1;
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

		if (Window* ctx = _getCurrentContext())
			ctx->RegisterFloatingPanel(m_stableId, this);

		m_child = SableMemory::SB_new<T>();
		m_child->SetRenderer(m_owner->GetRenderer());
	}

	template<typename T>
	inline FloatingPanel<T>::~FloatingPanel()
	{
		if (m_stableId >= 0)
			if (Window* ctx = _getCurrentContext())
				ctx->UnregisterFloatingPanel(m_stableId);

		// if m_stableId == -1, this means Sync() transferred ownership, no need to unregister

		if (m_child)
		{
			SableMemory::SB_delete(m_child);
			m_child = nullptr;
		}
	}

	template<typename T>
	inline void FloatingPanel<T>::Open(Rect rect)
	{
		if (m_open) return;

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
			entry.cmd = ctx->GetRenderer()->CreateSecondaryCommandBuffer();

			m_child->SetRenderer(ctx->GetRenderer());
			m_child->BackendInitialiseFloatingPanel(rect);

			entry.cmd.BeginRenderPass(entry.framebuffer);
			entry.cmd.Clear(0.0f, 0.0f, 0.0f, 0.0f);

			m_child->MarkDirty();
		}
		else if (needsGpuResize)
		{
			entry.cmd.EndRenderPass();
			mainCB.SetFramebufferSize(entry.framebuffer, rect.w, rect.h);
			m_child->GetRootElement()->SetRect(entry.cmd, rect);

			entry.cmd.Reset();
			entry.cmd.BeginRenderPass(entry.framebuffer);
			entry.cmd.Clear(0.0f, 0.0f, 0.0f, 0.0f);
			m_child->MarkDirty();
		}

		entry.pos = { rect.x, rect.y };
		entry.size = { rect.w, rect.h };
		entry.dirty = true;
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

		Window* ctx = _getCurrentContext();
		FloatingPanelEntry& entry = ctx->GetFloatingPanelEntry(m_stableId);
		CommandBuffer& mainCB = ctx->GetMainCommandBuffer();

		if (!entry.cmd.empty())
		{
			entry.cmd.EndRenderPass();
			entry.cmd.Reset();
		}

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
		entry.cmd = {};
		entry.size = {};
		entry.dirty = false;

		m_open = false;
		PostEmptyEvent();
	}

	template<typename T>
	inline void FloatingPanel<T>::Toggle(Rect rect)
	{
		m_open ? Close() : Open(rect);
	}

	template<typename T>
	inline bool FloatingPanel<T>::IsOpen() const
	{
		return m_open;
	}

	template<typename T>
	inline int FloatingPanel<T>::GetZIndex() const
	{
		return zIndex;
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

	template<typename T>
	inline void FloatingPanel<T>::HandleInput(const UIEventContext& ctx, int z)
	{
		if (m_child)
			m_child->HandleInput(ctx, z);
	}

	template<typename T>
	inline bool FloatingPanel<T>::CheckAndUpdate(const DrawableDrawData& externalDrawData)
	{
		if (!m_child) return false;
		FloatingPanelEntry& entry = _getCurrentContext()->GetFloatingPanelEntry(m_stableId);

		DrawableDrawData drData{
			entry.cmd,
			entry.framebuffer,
			{ entry.pos.x, entry.pos.y, entry.size.x, entry.size.y },
			externalDrawData.contextResources
		};

		bool changed = m_child->CheckAndUpdate(drData);
		if (changed)
			entry.dirty = true;

		return changed;
	}

	template<typename T>
	inline void FloatingPanel<T>::PostLayoutUpdate(const UIEventContext& ctx)
	{
		if (m_child)
			m_child->PostLayoutUpdate(ctx);
	}
}