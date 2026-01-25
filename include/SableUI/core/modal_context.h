#pragma once
#include <SableUI/core/component.h>
#include <SableUI/utils/string.h>
#include <SableUI/utils/utils.h>
#include <SableUI/styles/styles.h>
#include <SableUI/core/events.h>
#include <SableUI/SableUI.h>
#include <type_traits>

namespace SableUI
{
	class BaseModal : public BaseComponent
	{
	public:
		void Init(const SableString& str);
		void Layout();

		virtual void Header();
		virtual void Content();
		virtual void Footer() {}

		void OnUpdate(const UIEventContext& ctx);

	private:
		SableString title = "Call Init() to set title";
	};

	inline SableString GetModalID(const SableString& str)
	{
		return "_modal_" + str;
	}

	template <typename T>
	struct ModalContext
	{
		static_assert(std::is_base_of_v<BaseModal, T>,
			"ModalContext<T>: T must derive from BaseModal");

	public:
		ModalContext(const SableString& title)
		{
			m_title = title;
		}

		void Open()
		{
			ivec2 winSize = _getCurrentContext()->m_windowSize;
			FloatingPanelScoped(
				ref,
				T,
				GetModalID(m_title),
				Rect(0, 0, winSize.x, winSize.y),
				Style::bg(0, 0, 0, 0))
			{
				ref->Init(m_title);
			}
		}
		void Close()
		{
			QueueDestroyFloatingPanel(GetModalID(m_title));
		}
		bool IsOpen()
		{
			return IsFloatingPanelActive(GetModalID(m_title));
		}

	private:
		SableString m_title = "";
	};
}