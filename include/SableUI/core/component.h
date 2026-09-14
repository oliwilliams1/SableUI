#pragma once
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <SableUI/states/state_base.h>
#include <SableUI/core/drawable.h>
#include <SableUI/types/floating_panel_types.h>
#include <type_traits>
#include <vector>
#include <string>
#include <utility>

namespace SableUI
{
	class Window;
	class BaseComponent
	{
	public:
		BaseComponent(Colour colour = Colour{ 32, 32, 32 });
		static int GetNumInstances();
		virtual ~BaseComponent();

		virtual void Layout() {};
		virtual void OnUpdate(const UIUpdateContext& ctx) {};
		virtual void OnUpdatePostLayout(const UIUpdateContext& ctx) {};

		void LayoutWrapper();
		void BackendInitialisePanel();
		void BackendInitialiseChild(const std::string& name, BaseComponent* parent, const ElementInfo& info);
		void BackendInitialiseFloatingPanel(const Rect& rect, const ElementInfo& p_info = {});
		void SetRenderer(RendererBackend* renderer);
		RendererBackend* GetRenderer();
		void Render(const DrawableDrawData& drData, int z = 0);

		BaseComponent* AddComponent(const std::string& componentName);
		template <typename T>
		T* AddComponent();

		Element* GetRootElement();
		void SetRootElement(Element* element);
		int GetNumChildren() const;
		bool Rerender(const DrawableDrawData& drData, bool* hasContentsChanged = nullptr);

		void HandleInput(const UIInputState& ctx, int z);
		bool CheckAndUpdate(const DrawableDrawData& drData);
		void PostLayoutUpdate(const UIInputState& ctx, int z);

		void RegisterState(StateBase* state);
		void RegisterFloatingPanel(FloatingPanelBase* state);

		void MarkDirty();
		bool IsDirty() const { return needsRerender; }

		void CopyStateFrom(const BaseComponent& other);
		Element* GetElementById(const SableString& id);

		std::vector<BaseComponent*> m_componentChildren;

	protected:
		std::vector<BaseComponent*> m_garbageChildren;
		std::vector<StateBase*> m_states;
		std::vector<FloatingPanelBase*> m_floatingPanels;

	private:
		bool needsRerender = false;
		BaseComponent* AttachComponent(BaseComponent* component);
		UIInputState m_lastInputState;
		Element* m_rootElement = nullptr;
		size_t m_hash = 0;
		RendererBackend* m_renderer = nullptr;
		Colour m_bgColour = Colour{ 32, 32, 32 };
		int m_childCount = 0;
	};

	template <typename T>
	struct ComponentScope
	{
		static_assert(std::is_base_of_v<BaseComponent, T>,
			"ComponentScoped<T>: T must derive from BaseComponent");

	public:
		ComponentScope(BaseComponent* owner, std::string typeName, ElementInfo info)
			: m_owner(owner), m_typeName(std::move(typeName)), m_info(std::move(info))
		{
			m_child = owner->AddComponent<T>();
		}

		~ComponentScope()
		{
			if (m_child)
			{
				m_child->BackendInitialiseChild(
					m_typeName,
					m_owner,
					m_info
				);
			}
		}

		T* operator->()	noexcept { return m_child; }
		T& operator*()	noexcept { return *m_child; }
		T* get()		noexcept { return m_child; }

		ComponentScope(const ComponentScope&) = delete;
		ComponentScope& operator=(const ComponentScope&) = delete;
		ComponentScope(ComponentScope&& other) = delete;
		ComponentScope& operator=(ComponentScope&& other) = delete;

	private:
		T* m_child = nullptr;
		BaseComponent* m_owner = nullptr;
		std::string m_typeName;
		ElementInfo m_info;
	};

	template <typename T>
	inline T* BaseComponent::AddComponent()
	{
		static_assert(std::is_base_of_v<BaseComponent, T>, "AddComponent<T>: T must derive from BaseComponent");

		T* component = SableMemory::SB_new<T>();

		AttachComponent(component);
		return component;
	}

	void _priv_comp_PostEmptyEvent();
}

#include <SableUI/states/state.h>
#include <SableUI/states/ref.h>
#include <SableUI/states/interval.h>
#include <SableUI/states/timer.h>
#include <SableUI/states/floating_panel.h>
