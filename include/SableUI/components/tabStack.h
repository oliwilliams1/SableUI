#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <vector>
#include <string>

namespace SableUI
{
	struct _TabDef
	{
		std::string component;
		std::string label;
	};

	class TabStack : public SableUI::BaseComponent
	{
	public:
		void Layout() override;

		void AddTab(const std::string& componentName, const std::string& label);
		void AddTab(const std::string& componentName);

	private:
		useState(activeTab, setActiveTab, int, 0);
		useRef(tabs, std::vector<_TabDef>, {});
	};
}