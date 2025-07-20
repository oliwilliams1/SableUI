#pragma once
#include <memory>
#include <unordered_map>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
	class Renderer;

	enum class ElementType
	{
		UNDEF   = 0x0,
		RECT    = 0x1,
		IMAGE   = 0x2,
		TEXT    = 0x3,
		DIV     = 0x4,

	};

	enum class LayoutDirection
	{
		VERTICAL    = 0x0,
		HORIZONTAL  = 0x1
	};

	struct ElementInfo
	{
		std::string name;												// Custom name, used for attaching, editing components without a saved reference
		Colour bgColour = Colour(128, 128, 128);						// Background colour for solid rect elements
		int xOffset = 0;												// Horizontal offset from left side
		int yOffset = 0;												// Vertical offset from top
		int width = 0;												    // Width in px, only useful if wType == RectType::FIXED
		int height = 0;												    // Height in px, only useful if hType == RectType::FIXED
		int padding = 0;												// Inner padding of elements, useful when adding children elements
		bool centerX = false;											// Is centered horizontally?
		bool centerY = false;											// Is centered vertically?
		RectType wType = RectType::UNDEF;								// Can be FILL or FIXED, use fill to enable automatic dynamic resizing, fixed for fixed-width elements
		RectType hType = RectType::UNDEF;
		ElementType type = ElementType::UNDEF;							// Can be UNDEF (for init error checks), RECT (for solid rectangle elements), IMAGE, or TEXT
		LayoutDirection layoutDirection = LayoutDirection::VERTICAL;	// Vertical or horizontal child layout
	};

	class Element
	{
	public:
		Element(const std::string name, Renderer* renderer, ElementType type);
		~Element();

		void SetInfo(const ElementInfo& info);

		/* Base render (background) */
		void Render(int z = 1);

		void SetRect(const Rect& rect);

		void UpdateChildren();
		void AddChild(Element* child);
		void SetImage(const std::string& path);
		void SetText(const std::u32string& text, int fontSize = 11, float lineHeight = 1.15f);

		int GetWidth(bool surface = true);
		int GetHeight(bool surface = true);

		/* User-level settings for rect */
		int xOffset = 0;
		int yOffset = 0;
		int width = 0;
		int height = 0;
		int padding = 0;
		bool centerX = false;
		bool centerY = false;

		RectType wType = RectType::FILL;
		RectType hType = RectType::FILL;

		Colour bgColour = Colour(128, 128, 128);

		std::string name = "unnamed element";
		ElementType type = ElementType::UNDEF;
		std::vector<Element*> children;
		LayoutDirection layoutDirection = LayoutDirection::VERTICAL;

		Rect rect = { 0, 0, 0, 0 };
	private:
		/* Private vars for rendering */
		DrawableBase* drawable;
		Renderer* renderer = nullptr;
	};
}