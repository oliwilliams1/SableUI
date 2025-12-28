#pragma once
#include <SableUI/element.h>
#include <cstdint>
#include <functional>
#include <utility>
#include <SableUI/text.h>
#include <SableUI/utils.h>
#include <algorithm>
#include <cmath>

namespace SableUI
{
	inline constexpr SableUI::Colour rgb(uint8_t r, uint8_t g, uint8_t b) {
		return SableUI::Colour{ r, g, b, 255 };
	}

	inline constexpr SableUI::Colour rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		return SableUI::Colour{ r, g, b, a };
	}
}

namespace SableUI::Style
{
	template <typename T>
	struct Property {
		T value;
		void (*apply)(ElementInfo&, T);
		constexpr void ApplyTo(ElementInfo& info) const { apply(info, value); }
	};

	struct FlagProperty {
		void (*apply)(ElementInfo&);
		constexpr void ApplyTo(ElementInfo& info) const { apply(info); }
	};

	inline constexpr Property<const char*> id(const char* v) {
		return { v, [](ElementInfo& i, const char* val) { i.id = val; } };
	}
	inline Property<SableString> id(SableString v) {
		return { std::move(v), [](ElementInfo& i, SableString val) { i.id = std::move(val); } };
	}

	// sizing
	inline constexpr Property<int> w(int v) {
		return { v, [](ElementInfo& i, int val) { i.layout.width = val; i.layout.minW = val; i.layout.wType = RectType::Fixed; } };
	}
	inline constexpr Property<int> h(int v) {
		return { v, [](ElementInfo& i, int val) { i.layout.height = val; i.layout.minH = val; i.layout.hType = RectType::Fixed; } };
	}
	inline constexpr Property<int> minW(int v) { return { v, [](ElementInfo& i, int val) { i.layout.minW = val; } }; }
	inline constexpr Property<int> maxW(int v) { return { v, [](ElementInfo& i, int val) { i.layout.maxW = val; } }; }
	inline constexpr Property<int> minH(int v) { return { v, [](ElementInfo& i, int val) { i.layout.minH = val; } }; }
	inline constexpr Property<int> maxH(int v) { return { v, [](ElementInfo& i, int val) { i.layout.maxH = val; } }; }

	// rect type flags
	inline constexpr FlagProperty w_fill = { [](ElementInfo& i) { i.layout.wType = RectType::Fill; } };
	inline constexpr FlagProperty h_fill = { [](ElementInfo& i) { i.layout.hType = RectType::Fill; } };
	inline constexpr FlagProperty w_fixed = { [](ElementInfo& i) { i.layout.wType = RectType::Fixed; } };
	inline constexpr FlagProperty h_fixed = { [](ElementInfo& i) { i.layout.hType = RectType::Fixed; } };
	inline constexpr FlagProperty w_fit = { [](ElementInfo& i) { i.layout.wType = RectType::FitContent; } };
	inline constexpr FlagProperty h_fit = { [](ElementInfo& i) { i.layout.hType = RectType::FitContent; } };

	// alignment
	inline constexpr FlagProperty centerX = { [](ElementInfo& i) { i.layout.centerX = true; } };
	inline constexpr FlagProperty centerY = { [](ElementInfo& i) { i.layout.centerY = true; } };
	inline constexpr FlagProperty centerXY = { [](ElementInfo& i) { i.layout.centerX = i.layout.centerY = true; } };
	inline constexpr FlagProperty clipChildren = { [](ElementInfo& i) {i.appearance.clipChildren = true; }};

	// layout dirs
	inline constexpr Property<LayoutDirection> dir(LayoutDirection v) {
		return { v, [](ElementInfo& i, LayoutDirection val) { i.layout.layoutDirection = val; } };
	}
	inline constexpr FlagProperty left_right = { [](ElementInfo& i) { i.layout.layoutDirection = LayoutDirection::LeftRight; } };
	inline constexpr FlagProperty right_left = { [](ElementInfo& i) { i.layout.layoutDirection = LayoutDirection::RightLeft; } };
	inline constexpr FlagProperty up_down = { [](ElementInfo& i) { i.layout.layoutDirection = LayoutDirection::UpDown; } };
	inline constexpr FlagProperty down_up = { [](ElementInfo& i) { i.layout.layoutDirection = LayoutDirection::DownUp; } };

	// margins
	inline constexpr Property<int> m(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mT = i.layout.mB = i.layout.mL = i.layout.mR = val; } }; }
	inline constexpr Property<int> mx(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mL = i.layout.mR = val; } }; }
	inline constexpr Property<int> my(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mT = i.layout.mB = val; } }; }
	inline constexpr Property<int> mt(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mT = val; } }; }
	inline constexpr Property<int> mb(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mB = val; } }; }
	inline constexpr Property<int> ml(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mL = val; } }; }
	inline constexpr Property<int> mr(int v) { return { v, [](ElementInfo& i, int val) { i.layout.mR = val; } }; }

	// padding
	inline constexpr Property<int> p(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pT = i.layout.pB = i.layout.pL = i.layout.pR = val; } }; }
	inline constexpr Property<int> px(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pL = i.layout.pR = val; } }; }
	inline constexpr Property<int> py(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pT = i.layout.pB = val; } }; }
	inline constexpr Property<int> pt(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pT = val; } }; }
	inline constexpr Property<int> pb(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pB = val; } }; }
	inline constexpr Property<int> pl(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pL = val; } }; }
	inline constexpr Property<int> pr(int v) { return { v, [](ElementInfo& i, int val) { i.layout.pR = val; } }; }

	// gaps
	inline constexpr Property<int> gap(int v) { return { v, [](ElementInfo& i, int val) { i.layout.gap = val; } }; }
	inline constexpr Property<int> gapX(int v) { return { v, [](ElementInfo& i, int val) { i.layout.gapX = val; } }; }
	inline constexpr Property<int> gapY(int v) { return { v, [](ElementInfo& i, int val) { i.layout.gapY = val; } }; }

	// appearance
	inline constexpr Property<Colour> bg(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		return { Colour(r, g, b, a), [](ElementInfo& i, Colour val) { i.appearance.bg = val; } };
	}
	inline constexpr Property<Colour> bg(Colour c) {
		return { c, [](ElementInfo& i, Colour val) { i.appearance.bg = val; } };
	}
	inline constexpr Property<bool> inheritBg(bool v) {
		return { v, [](ElementInfo& i, bool val) { i.appearance.inheritBg = val; } };
	}
	inline constexpr Property<float> rounded(float r) {
		return { r, [](ElementInfo& i, float val) { i.appearance.radius = val; } };
	}
	inline constexpr FlagProperty overflow_hidden = { [](ElementInfo& i) { i.appearance.clipChildren = true; } };

	// sizes
	inline constexpr FlagProperty size_sm = { [](ElementInfo& i) {i.appearance.size = ComponentSize::Small; } };
	inline constexpr FlagProperty size_md = { [](ElementInfo& i) {i.appearance.size = ComponentSize::Medium; } };
	inline constexpr FlagProperty size_lg = { [](ElementInfo& i) {i.appearance.size = ComponentSize::Large; } };
	inline constexpr FlagProperty disabled = { [](ElementInfo& i) {i.appearance.disabled = true; } };
	
	// text
	inline constexpr Property<int> fontSize(int v) { return { v, [](ElementInfo& i, int val) { i.text.fontSize = val; } }; }
	inline constexpr Property<float> lineHeight(float v) { return { v, [](ElementInfo& i, float val) { i.text.lineHeight = val; } }; }
	inline constexpr Property<Colour> textColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		return { Colour(r, g, b, a), [](ElementInfo& i, Colour val) { i.text.colour = val; } };
	}
	inline constexpr Property<Colour> textColour(Colour c) {
		return { c, [](ElementInfo& i, Colour val) { i.text.colour = val; } };
	}
	inline constexpr Property<TextJustification> justify(TextJustification v) {
		return { v, [](ElementInfo& i, TextJustification val) { i.text.justification = val; } };
	}
	inline constexpr Property<bool> wrapText(bool v) {
		return { v, [](ElementInfo& i, bool val) { i.text.wrap = val; } };
	}

	inline constexpr FlagProperty justify_left = { [](ElementInfo& i) { i.text.justification = TextJustification::Left; } };
	inline constexpr FlagProperty justify_center = { [](ElementInfo& i) { i.text.justification = TextJustification::Center; } };
	inline constexpr FlagProperty justify_right = { [](ElementInfo& i) { i.text.justification = TextJustification::Right; } };

	// callbacks
	struct CallbackProperty {
		std::function<void()> func;
		void (*apply)(ElementInfo&, const std::function<void()>&);
		void ApplyTo(ElementInfo& i) const { apply(i, func); }
	};

	inline CallbackProperty onClick(std::function<void()> f) { return { std::move(f), [](ElementInfo& i, const std::function<void()>& v) { i.onClickFunc = v; } }; }
	inline CallbackProperty onSecondaryClick(std::function<void()> f) { return { std::move(f), [](ElementInfo& i, const std::function<void()>& v) { i.onSecondaryClickFunc = v; } }; }
	inline CallbackProperty onDoubleClick(std::function<void()> f) { return { std::move(f), [](ElementInfo& i, const std::function<void()>& v) { i.onDoubleClickFunc = v; } }; }

	// etc
	struct Pos { int x, y; };
	inline constexpr Property<Pos> absolutePos(int x, int y) {
		return { {x, y}, [](ElementInfo& i, Pos v) { i.layout.pos.x = v.x; i.layout.pos.y = v.y; } };
	}

	// hoverable
	inline constexpr Property<std::pair<Colour, Colour>> hoverBg(Colour normal, Colour hover) {
		return { {normal, hover}, [](ElementInfo& i, std::pair<Colour, Colour> val) {
			i.appearance.bg = val.first;
			i.appearance.hoverBg = val.second;
			i.appearance.hasHoverBg = true;
		} };
	}
}
