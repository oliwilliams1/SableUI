#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <any>
#include <string>

#include "SableUI/utils.h"
#include "SableUI/renderer.h"

namespace SableUI
{
	enum class MouseState
	{
		DOWN = 0x0000,
		UP = 0x0001
	};

	enum class MouseEvent
	{
		NONE = 0x0,
		CLICK = 0x1,
		RELEASE = 0x2
	};

	struct MouseButtonState
	{
		MouseState LMBState = MouseState::UP;
		MouseState RMBState = MouseState::UP;
		MouseEvent LMBEvent = MouseEvent::NONE;
		MouseEvent RMBEvent = MouseEvent::NONE;
		ivec2 pos			= ivec2(0, 0);
	};

	enum class ElementType
	{
		UNDEF = 0x0,
		RECT = 0x1,
		IMAGE = 0x2,
		TEXT = 0x3,
		DIV = 0x4,
		TEXT_U32 = 0x5
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
		std::string id;

		// attribs
		Colour bgColour = Colour(128, 128, 128);
		int width = 0;
		int height = 0;
		int minWidth = 0;
		int maxWidth = 0;
		int minHeight = 0;
		int maxHeight = 0;
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
		SableString textOrPath = "";

		std::function<void()> onHoverFunc = nullptr;
		std::function<void()> onHoverExitFunc = nullptr;
		std::function<void()> onClickFunc = nullptr;
		std::function<void()> onSecondaryClickFunc = nullptr;

		// setter functions for macros
		ElementInfo& setID(const std::string& v)			{ id = v; return *this; }
		ElementInfo& setBgColour(const Colour& v)			{ bgColour = v; return *this; }
		ElementInfo& setWidth(int v)						{ width = v; wType = RectType::FIXED; return *this; }
		ElementInfo& setMinWidth(int v)						{ minWidth = v; return *this; }
		ElementInfo& setMaxWidth(int v)						{ maxWidth = v; return *this; }
		ElementInfo& setHeight(int v)						{ height = v; hType = RectType::FIXED; return *this; }
		ElementInfo& setMinHeight(int v)					{ minHeight = v; return *this; }
		ElementInfo& setMaxHeight(int v)					{ maxHeight = v; return *this; }

		ElementInfo& setWType(RectType v)					{ wType = v; return *this; }
		ElementInfo& setHType(RectType v)					{ hType = v; return *this; }
		
		ElementInfo& setMargin(int v)						{ marginTop = v; marginBottom = v; marginLeft = v; marginRight = v; return *this; }
		ElementInfo& setMarginX(int v)						{ marginLeft = v; marginRight = v; return *this; }
		ElementInfo& setMarginY(int v)						{ marginTop = v; marginBottom = v; return *this; }
		ElementInfo& setMarginTop(int v)					{ marginTop = v; return *this; }
		ElementInfo& setMarginBottom(int v)					{ marginBottom = v; return *this; }
		ElementInfo& setMarginLeft(int v)					{ marginLeft = v; return *this; }
		ElementInfo& setMarginRight(int v)					{ marginRight = v; return *this; }
		
		ElementInfo& setPadding(int v)						{ paddingTop = v; paddingBottom = v; paddingLeft = v; paddingRight = v; return *this; }
		ElementInfo& setPaddingX(int v)						{ paddingLeft = v; paddingRight = v; return *this; }
		ElementInfo& setPaddingY(int v)						{ paddingTop = v; paddingBottom = v; return *this; }
		ElementInfo& setPaddingTop(int v)					{ paddingTop = v; return *this; }
		ElementInfo& setPaddingBottom(int v)				{ paddingBottom = v; return *this; }
		ElementInfo& setPaddingLeft(int v)					{ paddingLeft = v; return *this; }
		ElementInfo& setPaddingRight(int v)					{ paddingRight = v; return *this; }

		ElementInfo& setCenterX(bool v)						{ centerX = v; return *this; }
		ElementInfo& setCenterY(bool v)						{ centerY = v; return *this; }
		ElementInfo& setLayoutDirection(LayoutDirection v)	{ layoutDirection = v; return *this; }
		ElementInfo& setType(ElementType v)					{ type = v; return *this; }

		// Event functions
		ElementInfo& setOnHover(const std::function<void()>& func)			{ onHoverFunc			= func;	return *this; }
		ElementInfo& setOnHoverExit(const std::function<void()>& func)		{ onHoverExitFunc		= func;	return *this; }
		ElementInfo& setOnClick(const std::function<void()>& func)			{ onClickFunc			= func; return *this; }
		ElementInfo& setOnSecondaryClick(const std::function<void()>& func)	{ onSecondaryClickFunc	= func; return *this; }
	};

	class BaseComponent;
	struct VirtualNode
	{
		ElementType type;
		std::string key;
		std::vector<VirtualNode*> children;
		ElementInfo info;
		SableString uniqueTextOrPath;
		BaseComponent* childComp = nullptr;

		std::unordered_map<std::string, std::any> state;
	};

	enum class ChildType
	{
		ELEMENT = 0x0,
		COMPONENT = 0x1,
	};

	struct Child;

	class Element
	{
	public:
		Element();
		Element(Renderer* renderer, ElementType type);
		~Element();

		ElementType type = ElementType::UNDEF;
		
		// functions for engine
		void Init(Renderer* renderer, ElementType type);
		void SetInfo(const ElementInfo& info);
		void SetRect(const Rect& rect);
		void AddChild(Element* child);
		void AddChild(Child* component);
		void SetImage(const std::string& path);
		void SetText(const SableString& text, int fontSize = 11, float lineHeight = 1.15f);
		int GetMinWidth();
		int GetMinHeight();

		// user defined
		std::string ID = "";
		int width = 0;
		int height = 0;
		int minWidth = 0;
		int minHeight = 0;
		int maxWidth = 0;
		int maxHeight = 0;
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
		SableString uniqueTextOrPath = "";

		std::function<void()> m_onHoverFunc = nullptr;
		std::function<void()> m_onHoverExitFunc = nullptr;
		std::function<void()> m_onClickFunc = nullptr;
		std::function<void()> m_onSecondaryClickFunc = nullptr;
		
		// setter functions for macros
		Element& setID(const std::string& v)			{ ID = v; return *this; }
		Element& setBgColour(const Colour& v)			{ bgColour = v; return *this; }
		Element& setWidth(int v)						{ width = v; wType = RectType::FIXED; return *this; }
		Element& setMinWidth(int v)						{ minWidth = v; return *this; }
		Element& setMaxWidth(int v)						{ maxWidth = v; return *this; }
		Element& setHeight(int v)						{ height = v; hType = RectType::FIXED; return *this; }
		Element& setMinHeight(int v)					{ minHeight = v; return *this; }
		Element& setMaxHeight(int v)					{ maxHeight = v; return *this; }
		
		Element& setWType(RectType v)					{ wType = v; return *this; }
		Element& setHType(RectType v)					{ hType = v; return *this; }

		Element& setMargin(int v)						{ marginTop = v; marginBottom = v; marginLeft = v; marginRight = v; return *this; }
		Element& setMarginX(int v)						{ marginLeft = v; marginRight = v; return *this; }
		Element& setMarginY(int v)						{ marginTop = v; marginBottom = v; return *this; }
		Element& setMarginTop(int v)					{ marginTop = v; return *this; }
		Element& setMarginBottom(int v)					{ marginBottom = v; return *this; }
		Element& setMarginLeft(int v)					{ marginLeft = v; return *this; }
		Element& setMarginRight(int v)					{ marginRight = v; return *this; }

		Element& setPadding(int v)						{ paddingTop = v; paddingBottom = v; paddingLeft = v; paddingRight = v; return *this; }
		Element& setPaddingX(int v)						{ paddingLeft = v; paddingRight = v; return *this; }
		Element& setPaddingY(int v)						{ paddingTop = v; paddingBottom = v; return *this; }
		Element& setPaddingTop(int v)					{ paddingTop = v; return *this; }
		Element& setPaddingBottom(int v)				{ paddingBottom = v; return *this; }
		Element& setPaddingLeft(int v)					{ paddingLeft = v; return *this; }
		Element& setPaddingRight(int v)					{ paddingRight = v; return *this; }

		Element& setCenterX(bool v)						{ centerX = v; return *this; }
		Element& setCenterY(bool v)						{ centerY = v; return *this; }
		Element& setLayoutDirection(LayoutDirection v)	{ layoutDirection = v; return *this; }

		Element& setType(ElementType v) { type = v; return *this; }

		Element& setOnHover(const std::function<void()>& func)			{ m_onHoverFunc				= func;	return *this; }
		Element& setOnHoverExit(const std::function<void()>& func)		{ m_onHoverExitFunc			= func;	return *this; }
		Element& setOnClick(const std::function<void()>& func)			{ m_onClickFunc				= func; return *this; }
		Element& setOnSecondaryClick(const std::function<void()>& func) { m_onSecondaryClickFunc	= func; return *this; }

		/* internal functions */
		bool Reconcile(VirtualNode* vnode);
		void BuildRealSubtreeFromVirtual(VirtualNode* vnode, Element* parent);

		// event system
		bool el_PropagateComponentStateChanges();
		void HandleHoverEvent(const ivec2& mousePos);
		void HandleMouseClickEvent(const MouseButtonState& mouseState);

		// rendering
		void Render(int z = 1);
		Rect rect = { 0, 0, 0, 0 };

		// children handling
		void LayoutChildren();
		bool layoutDirty = false;
		int measuredHeight = 0;
		std::vector<Child*> children;

	private:
		bool isHovered = false;
		DrawableBase* drawable = nullptr;
		Renderer* renderer = nullptr;
	};

	struct Child
	{
		ChildType type = ChildType::ELEMENT;
		union
		{
			BaseComponent* component;
			Element* element;
		};

		Child(Element* element) : element(element), type(ChildType::ELEMENT) {}
		Child(BaseComponent* component) : component(component), type(ChildType::COMPONENT) {}

		~Child();

		operator SableUI::Element* ();
	};
}