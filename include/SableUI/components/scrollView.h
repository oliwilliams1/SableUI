#pragma once
#include <SableUI/component.h>
#include <SableUI/SableUI.h>
#include <SableUI/element.h>
#include <SableUI/console.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <type_traits>
#include <functional>
#include <algorithm>
#include <utility>
#include <string>

namespace SableUI
{
	struct ScrollData {
		ScrollData() = default;
		ScrollData(const ivec2& viewportSize, const ivec2& contentSize)
			: viewportSize(viewportSize), contentSize(contentSize) {};
		vec2 viewportSize = { 0, 0 };
		vec2 contentSize = { 0, 0 };

		bool operator==(const ScrollData& other) const
		{
			return viewportSize == other.viewportSize && contentSize == other.contentSize;
		};
	};

	class ScrollView : public SableUI::BaseComponent
	{
	public:
		void AttachChild(const std::string& p_childID, const ElementInfo& childInfo = ElementInfo{});
		template <typename T>
		void AttachChildWithInitialiser(const std::string& p_childID,
			std::function<void(T*)> initialiser,
			const ElementInfo& childInfo = ElementInfo{});

		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		const int scrollMultiplier = 50;
		useState(barHovered, setBarHovered, bool, false);
		useState(scrollData, setScrollData, ScrollData, {});
		useState(scrollPos, setScrollPos, vec2, { 0, 0 });
		useState(isDragging, setIsDragging, bool, false);
		useState(dragOrigPos, setDragOrigPos, ivec2, { 0, 0 });
		useState(dragStartScrollY, setDragStartScrollY, float, 0.0f);
		std::string childID = "";
		std::function<void(BaseComponent*)> childInitialiser;
		ElementInfo childElInfo{};
	};

	template <typename T>
	inline void ScrollView::AttachChildWithInitialiser(const std::string& p_childID,
		std::function<void(T*)> initialiser,
		const ElementInfo& childInfo)
	{
		static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");
		
		if (!childID.empty())
		{
			SableUI_Warn("Scrollview cannot have more than 1 component, skipping addition of %s", p_childID.c_str());
			return;
		}
		
		childInitialiser = [init = std::move(initialiser)](BaseComponent* comp) {
			if (T* derived = dynamic_cast<T*>(comp)) {
				init(derived);
			}
			else {
				SableUI_Error("Component type mismatch in initialiser for '%s", typeid(T).name());
			}
		};

		this->childID = p_childID;
		this->childElInfo = childInfo;
		needsRerender = true;
	}
}