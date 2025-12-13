#pragma once
#include <SableUI/SableUI.h>
#include <SableUI/component.h>
#include <SableUI/events.h>
#include <SableUI/utils.h>
#include <SableUI/element.h>
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
		void Layout() override;
		void OnUpdate(const UIEventContext& ctx) override;

	private:
		const int scrollMultiplier = 100;
		useState(barHovered, setBarHovered, bool, false);
		useState(scrollData, setScrollData, ScrollData, {});
		useState(scrollPos, setScrollPos, vec2, { 0, 0 });
		useState(isDragging, setIsDragging, bool, false);
		useState(dragOrigPos, setDragOrigPos, ivec2, { 0, 0 });
		useState(dragStartScrollY, setDragStartScrollY, float, 0.0f);
		std::string childID = "";
		ElementInfo childElInfo{};
	};
}