#pragma once
#include <SableUI/core/component.h>
#include <SableUI/states/state_base.h>
#include <SableUI/states/floating_panel_base.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/events.h>
#include <type_traits>

namespace SableUI
{
	class BaseComponent;
	template <typename T>
	class FloatingPanelState : public FloatingPanelStateBase
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

		T* Initialise();
		void Finalise();

		void HandleInput(const UIEventContext& ctx) override;
		void Layout();
		void PostLayoutUpdate(const UIEventContext& ctx);

		bool IsOpen() const override;

		void Sync(StateBase* other) override;

	private:
		bool m_open = false;
		T* m_child = nullptr;
		GpuFramebuffer m_framebuffer;
		GpuTexture2D m_colourAttachment;
		BaseComponent* m_owner = nullptr;
		Rect m_rect{};
	};
}

template <typename T>
inline SableUI::FloatingPanelState<T>::FloatingPanelState(BaseComponent* owner) {
	m_owner = owner;

	if (m_child != nullptr)
		SableMemory::SB_delete(m_child);

	T* comp = SableMemory::SB_new<T>();
	comp->SetRenderer(m_owner->GetRenderer());

	m_owner->RegisterFloatingPanel(this);
}

template <typename T>
inline void SableUI::FloatingPanelState<T>::Sync(StateBase* other) {
	auto* otherPtr = static_cast<FloatingPanelState<T>*>(other);
	m_open = otherPtr->m_open;
	m_framebuffer = otherPtr->m_framebuffer;
	m_colourAttachment = otherPtr->m_colourAttachment;
	m_rect = otherPtr->m_rect;
	m_child->CopyStateFrom(*otherPtr->m_child);
}

template<typename T>
inline SableUI::FloatingPanelState<T>::~FloatingPanelState() { SableMemory::SB_delete(m_child); }

template<typename T>
inline void SableUI::FloatingPanelState<T>::Toggle() { m_open = !m_open; }

template<typename T>
inline void SableUI::FloatingPanelState<T>::Open() { m_open = true; }

template<typename T>
inline void SableUI::FloatingPanelState<T>::Close() { m_open = false; }

template<typename T>
inline void SableUI::FloatingPanelState<T>::SetPosition(int x, int y) { m_rect.x = x; m_rect.y = y; }

template<typename T>
inline void SableUI::FloatingPanelState<T>::SetSize(int width, int height) { m_rect.width = width; m_rect.height = height; }

template<typename T>
inline void SableUI::FloatingPanelState<T>::SetRect(const Rect& rect) { m_rect = rect; }

template<typename T>
inline T* SableUI::FloatingPanelState<T>::Initialise()
{
	if (m_child)
		SableMemory::SB_delete(m_child);

	T* component = SableMemory::SB_new<T>();

	return component;
}

template<typename T>
inline void SableUI::FloatingPanelState<T>::Finalise()
{
	if (!m_child)
		return;

	m_child->BackendInitialiseFloatingPanel(m_rect, m_info);
}

template<typename T>
inline void SableUI::FloatingPanelState<T>::HandleInput(const UIEventContext& ctx) { if (m_child) m_child->HandleInput(ctx); }

template<typename T>
inline void SableUI::FloatingPanelState<T>::Layout() { if (m_child) m_child->LayoutWrapper(); }

template<typename T>
inline void SableUI::FloatingPanelState<T>::PostLayoutUpdate(const UIEventContext& ctx) { if (m_child) m_child->PostLayoutUpdate(ctx); }

template<typename T>
inline bool SableUI::FloatingPanelState<T>::IsOpen() const { return m_open; }
