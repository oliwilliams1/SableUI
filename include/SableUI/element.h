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
		UP_DOWN    = 0x0,
		DOWN_UP    = 0x1,
		LEFT_RIGHT = 0x2,
		RIGHT_LEFT = 0x3,
	};

	struct ElementInfo
	{
		std::string ID;

		// attribs
		Colour bgColour = Colour(128, 128, 128);
		int width = 0;
		int height = 0;
		int marginTop = 0;
		int marginBottom = 0;
		int marginLeft = 0;
		int marginRight = 0;
		int paddingTop = 0;
		int paddingBottom = 0;
		int paddingLeft = 0;
		int paddingRight = 0;
		bool centerX = false;
		bool centerY = false;
		RectType wType = RectType::FILL;
		RectType hType = RectType::FILL;
		ElementType type = ElementType::UNDEF;
		LayoutDirection layoutDirection = LayoutDirection::UP_DOWN;

		// setter functions for macros
		ElementInfo& setID(const std::string& v) { ID = v; return *this; }
		ElementInfo& setBgColour(const Colour& v) { bgColour = v; return *this; }
		ElementInfo& setWidth(int v) { width = v; wType = RectType::FIXED; return *this; }
		ElementInfo& setHeight(int v) { height = v; hType = RectType::FIXED; return *this; }
		ElementInfo& setWType(RectType v) { wType = v; return *this; }
		ElementInfo& setHType(RectType v) { hType = v; return *this; }
		
		ElementInfo& setMargin(int v) { marginTop = v; marginBottom = v; marginLeft = v; marginRight = v; return *this; }
		ElementInfo& setMarginX(int v) { marginLeft = v; marginRight = v; return *this; }
		ElementInfo& setMarginY(int v) { marginTop = v; marginBottom = v; return *this; }
		ElementInfo& setMarginTop(int v) { marginTop = v; return *this; }
		ElementInfo& setMarginBottom(int v) { marginBottom = v; return *this; }
		ElementInfo& setMarginLeft(int v) { marginLeft = v; return *this; }
		ElementInfo& setMarginRight(int v) { marginRight = v; return *this; }
		
		ElementInfo& setPadding(int v) { paddingTop = v; paddingBottom = v; paddingLeft = v; paddingRight = v; return *this; }
		ElementInfo& setPaddingX(int v) { paddingLeft = v; paddingRight = v; return *this; }
		ElementInfo& setPaddingY(int v) { paddingTop = v; paddingBottom = v; return *this; }
		ElementInfo& setPaddingTop(int v) { paddingTop = v; return *this; }
		ElementInfo& setPaddingBottom(int v) { paddingBottom = v; return *this; }
		ElementInfo& setPaddingLeft(int v) { paddingLeft = v; return *this; }
		ElementInfo& setPaddingRight(int v) { paddingRight = v; return *this; }

		ElementInfo& setCenterX(bool v) { centerX = v; return *this; }
		ElementInfo& setCenterY(bool v) { centerY = v; return *this; }
		ElementInfo& setLayoutDirection(LayoutDirection v) { layoutDirection = v; return *this; }

		ElementInfo& setType(ElementType v) { type = v; return *this; }
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

		ElementType type = ElementType::UNDEF;
		
		// functions for engine
		void Init(Renderer* renderer, ElementType type);
		void SetInfo(const ElementInfo& info);
		void Render(int z = 1);
		void SetRect(const Rect& rect);
		void AddChild(Element* child);
		void AddChild(BaseComponent* component);
		void SetImage(const std::string& path);
		void SetText(const std::u32string& text, int fontSize = 11, float lineHeight = 1.15f);

		int GetMinWidth();
		int GetMinHeight();

		// user defined
		std::string ID = "";
		int width = 0;
		int height = 0;
		int marginTop = 0;
		int marginBottom = 0;
		int marginLeft = 0;
		int marginRight = 0;
		int paddingTop = 0;
		int paddingBottom = 0;
		int paddingLeft = 0;
		int paddingRight = 0;
		bool centerX = false;
		bool centerY = false;
		RectType wType = RectType::FILL;
		RectType hType = RectType::FILL;
		Colour bgColour = Colour(128, 128, 128);
		LayoutDirection layoutDirection = LayoutDirection::UP_DOWN;

		// children handling
		void LayoutChildren();
		std::vector<Child> children;

		Rect rect = { 0, 0, 0, 0 }; // data for rendering

		// setter functions for macros
		Element& setID(const std::string& v) { ID = v; return *this; }
		Element& setBgColour(const Colour& v) { bgColour = v; return *this; }
		Element& setWidth(int v) { width = v; wType = RectType::FIXED; return *this; }
		Element& setHeight(int v) { height = v; hType = RectType::FIXED; return *this; }
		Element& setWType(RectType v) { wType = v; return *this; }
		Element& setHType(RectType v) { hType = v; return *this; }

		Element& setMargin(int v) { marginTop = v; marginBottom = v; marginLeft = v; marginRight = v; return *this; }
		Element& setMarginX(int v) { marginLeft = v; marginRight = v; return *this; }
		Element& setMarginY(int v) { marginTop = v; marginBottom = v; return *this; }
		Element& setMarginTop(int v) { marginTop = v; return *this; }
		Element& setMarginBottom(int v) { marginBottom = v; return *this; }
		Element& setMarginLeft(int v) { marginLeft = v; return *this; }
		Element& setMarginRight(int v) { marginRight = v; return *this; }

		Element& setPadding(int v) { paddingTop = v; paddingBottom = v; paddingLeft = v; paddingRight = v; return *this; }
		Element& setPaddingX(int v) { paddingLeft = v; paddingRight = v; return *this; }
		Element& setPaddingY(int v) { paddingTop = v; paddingBottom = v; return *this; }
		Element& setPaddingTop(int v) { paddingTop = v; return *this; }
		Element& setPaddingBottom(int v) { paddingBottom = v; return *this; }
		Element& setPaddingLeft(int v) { paddingLeft = v; return *this; }
		Element& setPaddingRight(int v) { paddingRight = v; return *this; }

		Element& setCenterX(bool v) { centerX = v; return *this; }
		Element& setCenterY(bool v) { centerY = v; return *this; }
		Element& setLayoutDirection(LayoutDirection v) { layoutDirection = v; return *this; }

		Element& setType(ElementType v) { type = v; return *this; }

	private:
		DrawableBase* drawable = nullptr;
		Renderer* renderer = nullptr;
	};
}