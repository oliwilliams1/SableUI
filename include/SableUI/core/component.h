#pragma once
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/element.h>
#include <SableUI/core/events.h>
#include <SableUI/utils/utils.h>
#include <SableUI/utils/memory.h>
#include <SableUI/states/state_base.h>
#include <type_traits>
#include <vector>
#include <string>
#include <utility>

namespace SableUI
{
	class FloatingPanelStateBase;
	class Window;
	class BaseComponent
	{
	public:
		BaseComponent(Colour colour = Colour{ 32, 32, 32 });
		static int GetNumInstances();
		virtual ~BaseComponent();

		virtual void Layout() {};
		virtual void OnUpdate(const UIEventContext& ctx) {};
		virtual void OnUpdatePostLayout(const UIEventContext& ctx) {};

		void LayoutWrapper();
		void BackendInitialisePanel();
		void BackendInitialiseChild(const std::string& name, BaseComponent* parent, const ElementInfo& info);
		void BackendInitialiseFloatingPanel(const Rect& rect, const ElementInfo& p_info = {});
		void SetRenderer(RendererBackend* renderer);
		RendererBackend* GetRenderer();
		void Render(CommandBuffer& cmd, const GpuFramebuffer* framebuffer, ContextResources& contextResources, int z = 0);

		BaseComponent* AddComponent(const std::string& componentName);
		template <typename T>
		T* AddComponent();

		Element* GetRootElement();
		void SetRootElement(Element* element);
		int GetNumChildren() const;
		bool Rerender(CommandBuffer& cmd, const GpuFramebuffer* framebuffer, ContextResources& contextResources, bool* hasContentsChanged = nullptr);

		void HandleInput(const UIEventContext& ctx);
		bool CheckAndUpdate(CommandBuffer& cmd, const GpuFramebuffer* framebuffer, ContextResources& contextResources);
		void PostLayoutUpdate(const UIEventContext& ctx);

		void RegisterState(StateBase* state);
		void RegisterFloatingPanel(FloatingPanelStateBase* state);

		void MarkDirty();
		bool IsDirty() const { return needsRerender; }

		void CopyStateFrom(const BaseComponent& other);
		Element* GetElementById(const SableString& id);

		std::vector<BaseComponent*> m_componentChildren;
		void RegisterHoverElement(Element* el);

	protected:
		std::vector<BaseComponent*> m_garbageChildren;
		std::vector<StateBase*> m_states;
		std::vector<FloatingPanelStateBase*> m_floatingPanels;
		std::vector<Element*> m_hoverElements;

		void UpdateHoverStyling(const UIEventContext& ctx);

	private:
		bool needsRerender = false;
		BaseComponent* AttachComponent(BaseComponent* component);
		UIEventContext m_lastEventCtx;
		Element* rootElement = nullptr;
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