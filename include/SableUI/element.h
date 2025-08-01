#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
	enum class ElementType
	{
		UNDEF = 0x0,
		RECT = 0x1,
		IMAGE = 0x2,
		TEXT = 0x3,
		DIV = 0x4,
	};

	enum class LayoutDirection
	{
		VERTICAL = 0x0,
		HORIZONTAL = 0x1
	};

	struct ElementInfo
	{
		std::string name;
		Colour bgColour = Colour(128, 128, 128);
		int xOffset = 0;
		int yOffset = 0;
		int width = 0;
		int height = 0;
		int paddingX = 0;
		int paddingY = 0;
		bool centerX = false;
		bool centerY = false;
		RectType wType = RectType::UNDEF;
		RectType hType = RectType::UNDEF;
		ElementType type = ElementType::UNDEF;
		LayoutDirection layoutDirection = LayoutDirection::VERTICAL;

		ElementInfo& setName(const std::string& newName) { name = newName; return *this; }
		ElementInfo& setBgColour(const Colour& color) { bgColour = color; return *this; }
		ElementInfo& setXOffset(int offset) { xOffset = offset; return *this; }
		ElementInfo& setYOffset(int offset) { yOffset = offset; return *this; }
		ElementInfo& setWidth(int newWidth) { width = newWidth; return *this; }
		ElementInfo& setHeight(int newHeight) { height = newHeight; return *this; }
		ElementInfo& setPaddingX(int newPadding) { paddingX = newPadding; return *this; }
		ElementInfo& setPaddingY(int newPadding) { paddingY = newPadding; return *this; }
		ElementInfo& setCenterX(bool value) { centerX = value; return *this; }
		ElementInfo& setCenterY(bool value) { centerY = value; return *this; }
		ElementInfo& setWType(RectType newType) { wType = newType; return *this; }
		ElementInfo& setHType(RectType newType) { hType = newType; return *this; }
		ElementInfo& setType(ElementType newType) { type = newType; return *this; }
		ElementInfo& setLayoutDirection(LayoutDirection newDirection) { layoutDirection = newDirection; return *this; }
	};

	enum class ChildType
	{
		ELEMENT = 0x0,
		COMPONENT = 0x1,
	};

	class BaseComponent;
	struct Child
	{
		ChildType type = ChildType::ELEMENT;
		union
		{
			BaseComponent* component;
			class Element* element;
		};

		Child(Element* element) : element(element), type(ChildType::ELEMENT) {}
		Child(BaseComponent* component) : component(component), type(ChildType::COMPONENT) {}

		operator SableUI::Element* ();
	};

	class Element
	{
	public:
		Element() {}
		Element(Renderer* renderer, ElementType type);
		~Element();

		void Init(Renderer* renderer, ElementType type);
		void SetInfo(const ElementInfo& info);
		void Render(int z = 1);
		void SetRect(const Rect& rect);
		void AddChild(Element* child);
		void AddChild(BaseComponent* component);
		void SetImage(const std::string& path);
		void SetText(const std::u32string& text, int fontSize = 11, float lineHeight = 1.15f);

		int xOffset = 0;
		int yOffset = 0;
		int width = 0;
		int height = 0;
		int paddingX = 0;
		int paddingY = 0;
		bool centerX = false;
		bool centerY = false;

		RectType wType = RectType::FILL;
		RectType hType = RectType::FILL;

		Colour bgColour = Colour(128, 128, 128);
		ElementType type = ElementType::UNDEF;
		std::vector<Child> children;
		LayoutDirection layoutDirection = LayoutDirection::VERTICAL;

		Rect rect = { 0, 0, 0, 0 };

	private:
		DrawableBase* drawable = nullptr;
		Renderer* renderer = nullptr;
	};
}