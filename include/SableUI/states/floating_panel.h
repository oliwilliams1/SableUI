#pragma once
#include <SableUI/core/component.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/events.h>
#include <type_traits>

namespace SableUI
{
	template <typename T>
	class FloatingPanelState : public StateBase
	{
		static_assert(std::is_base_of_v<BaseComponent, T>,
			"FloatingPanelState<T>: T must derive from BaseComponent");

	public:
		FloatingPanelState(BaseComponent* owner);
		~FloatingPanelState();

		void Toggle();
		void Open();
		void Close();

		void SetPosition(int x, int y);
		void SetSize(int width, int height);
		void SetRect(const Rect& rect);

		void OnUpdate(const UIEventContext& ctx);
		void Layout();
		void OnUpdatePostLayout(const UIEventContext& ctx);

		void Sync(StateBase* other) override;

	private:
		bool m_open = false;
		T* m_child = nullptr;
		GpuFramebuffer m_framebuffer;
		GpuTexture2D m_colourAttachment;
		BaseComponent* m_owner = nullptr;
		Rect m_rect{};
	};

	template <typename T>
	inline FloatingPanelState<T>::FloatingPanelState(BaseComponent* owner) {
		m_owner = owner;

		if (m_child != nullptr)
			SableMemory::SB_delete(m_child);

		T* comp = SableMemory::SB_new<T>();
		comp->SetRenderer(m_owner->GetRenderer());
	}

	template <typename T>
	inline void FloatingPanelState<T>::Sync(StateBase* other) {
		auto* otherPtr = static_cast<FloatingPanelState<T>*>(other);
		m_open = otherPtr->m_open;
		m_framebuffer = otherPtr->m_framebuffer;
		m_colourAttachment = otherPtr->m_colourAttachment;
		m_rect = otherPtr->m_rect;
		m_child->CopyStateFrom(*otherPtr->m_child);
	}

	template<typename T>
	inline FloatingPanelState<T>::~FloatingPanelState() { SableMemory::SB_delete(m_child); }

	template<typename T>
	inline void FloatingPanelState<T>::Toggle() { m_open = !m_open; }

	template<typename T>
	inline void FloatingPanelState<T>::Open() { m_open = true; }

	template<typename T>
	inline void FloatingPanelState<T>::Close() { m_open = false; }

	template<typename T>
	inline void FloatingPanelState<T>::SetPosition(int x, int y) { m_rect.x = x; m_rect.y = y; }

	template<typename T>
	inline void FloatingPanelState<T>::SetSize(int width, int height) { m_rect.width = width; m_rect.height = height; }

	template<typename T>
	inline void FloatingPanelState<T>::SetRect(const Rect& rect) { m_rect = rect; }

	template<typename T>
	inline void FloatingPanelState<T>::OnUpdate(const UIEventContext& ctx) { if (m_child) m_child->OnUpdate(ctx); }

	template<typename T>
	inline void FloatingPanelState<T>::OnUpdatePostLayout(const UIEventContext& ctx) { if (m_child) m_child->OnUpdatePostLayout(ctx); }
}