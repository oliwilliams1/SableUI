#pragma once
#include <vector>
#include <type_traits>
#include <SableUI/component.h>
#include <SableUI/memory.h>
#include <SableUI/SableUI.h>
#include <SableUI/panel.h>
#include <SableUI/events.h>

namespace SableUI
{
	class TabStackPanel : public SableUI::BaseComponent
	{
	public:
		~TabStackPanel();
		void Layout() override;

		template<typename T, typename... Args>
		void AddTab(Args&&... args);

		void OnUpdate(const UIEventContext& ctx) override;
		void comp_PropagateEvents(const UIEventContext& ctx) override;
		bool comp_PropagateComponentStateChanges(bool* changed) override;

	private:
		useState(activeTab, setActiveTab, int, 0);
		std::vector<BaseComponent*> tabs;
	};

	void StartTabStack(TabStackPanel* ref);
	void EndTabStack();
	TabStackPanel* GetTabReference();

	struct TabStackScope
	{
	public:
		TabStackScope(TabStackPanel* ref)
		{
			SableUI::StartTabStack(ref);
		};

		~TabStackScope()
		{
			SableUI::EndTabStack();
		}
		TabStackScope(const TabStackScope&) = delete;
		TabStackScope& operator=(const TabStackScope&) = delete;
		TabStackScope(TabStackScope&&) = default;
		TabStackScope& operator=(TabStackScope&&) = default;
	};

	template<typename T, typename... Args>
	inline void TabStackPanel::AddTab(Args&&... args)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from base component");
		T* newComp = SableMemory::SB_new<T>(std::forward<Args>(args)...);
		newComp->BackendInitialiseChild(STRINGIFY(T), this, style());
		tabs.push_back(newComp);
	}

	inline TabStackPanel::~TabStackPanel()
	{
		for (BaseComponent* tab : tabs)
			SableMemory::SB_delete(tab);

		tabs.clear();
	}
}

#define TabStack()																								\
	SableUI::TabStackPanel* CONCAT(_tab_stack_ref_, __LINE__) = nullptr;										\
	SableUI::ContentPanel* CONCAT(_panel_, __LINE__) = SableUI::AddPanel();										\
	CONCAT(_tab_stack_ref_, __LINE__) = CONCAT(_panel_, __LINE__)->AttachComponent<SableUI::TabStackPanel>();	\
	if (SableUI::TabStackScope CONCAT(_tab_guard_, __LINE__)(CONCAT(_tab_stack_ref_, __LINE__)); true)

#define TabItem(T, ...)						SableUI::GetTabReference()->AddTab<T>(__VA_ARGS__)