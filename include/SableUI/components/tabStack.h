#pragma once
#include <SableUI/component.h>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <SableUI/console.h>

namespace SableUI
{
	struct _TabDef
	{
		std::string component;
		std::string label;
		std::function<void(BaseComponent*)> initialiser;
	};

	class _TabStackDef : public SableUI::BaseComponent
	{
	public:
		void Layout() override;

		template <typename T>
		void AddTabWithInitialiser(
			const std::string& componentName, 
			const std::string& label, 
			std::function<void(T*)> initialiser);

		void AddTab(const std::string& componentName, const std::string& label);
		void AddTab(const std::string& componentName);

	private:
		void ValidateRegistration(const std::string componentName);
		bool TabExists(const std::string& label);
		int activeTab = 0; SableUI::StateSetter<int> setActiveTab = SableUI::StateSetter<int>([this](int const& val) { if (this->activeTab == val) return; this->activeTab = val; this->needsRerender = true; }); struct __StateReg_activeTab {
			__StateReg_activeTab(SableUI::BaseComponent* comp, int* var) {
				if (comp) comp->RegisterState(var);
			}
		} __stateReg_activeTab{ this, &activeTab };
		std::vector<_TabDef> tabs = {}; struct __RefReg_tabs {
			__RefReg_tabs(SableUI::BaseComponent* comp, std::vector<_TabDef>* var) {
				if (comp) comp->RegisterReference(var);
			}
		} __refReg_tabs{ this, &tabs };
		// ^^ Expanded from useState and useRef to remove circular dependency
	};

	template <typename T>
	inline void _TabStackDef::AddTabWithInitialiser(
		const std::string& componentName,
		const std::string& label,
		std::function<void(T*)> initialiser)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");
		ValidateRegistration(componentName);

		if (TabExists(label))
		{
			SableUI_Warn("Tab with label '%s' already exists, will be skipped", label.c_str());
			return;
		}

		_TabDef tabDef;
		tabDef.component = componentName;
		tabDef.label = label;
		tabDef.initialiser = [init = std::move(initialiser)](BaseComponent* comp) {
			if (T* derived = dynamic_cast<T*>(comp)) {
				init(derived);
			}
			else {
				SableUI_Error("Component type mismatch in initializer for '%s'", typeid(T).name());
			}
		};

		tabs.push_back(std::move(tabDef));
		needsRerender = true;
	}
}