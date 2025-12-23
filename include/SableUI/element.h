#pragma once
#include <vector>
#include <string>
#include <functional>

#include <SableUI/renderer.h>
#include <SableUI/events.h>
#include <SableUI/drawable.h>
#include <SableUI/text.h>
#include <SableUI/utils.h>

namespace SableUI
{
	enum class ElementType
	{
		Undef = 0x0,
		RECT = 0x1,
		IMAGE = 0x2,
		TEXT = 0x3,
		DIV = 0x4
	};

	enum class LayoutDirection
	{
		UP_DOWN = 0x0,
		DOWN_UP = 0x1,
		LEFT_RIGHT = 0x2,
		RIGHT_LEFT = 0x3,
	};

	struct LayoutProps {
		RectType wType = RectType::FitContent;
		RectType hType = RectType::FitContent;
		LayoutDirection layoutDirection = LayoutDirection::UP_DOWN;
		int width = 0, height = 0;
		int minW = 0, maxW = 0, minH = 0, maxH = 0;
		int pT = 0, pB = 0, pL = 0, pR = 0;
		int mT = 0, mB = 0, mL = 0, mR = 0;
		bool centerX = false, centerY = false;
		bool clipChildren;
	};

	struct AppearanceProps {
		Colour bg = { 0, 0, 0, 0 };
		float radius = 0.0f;
		bool clipChildren = false;
	};

	struct TextProps {
		SableString text;
		Colour colour = { 255, 255, 255, 255 };
		TextJustification justification = TextJustification::Left;
		int fontSize = 11;
		float lineHeight = 1.15f;
		bool wrap = true;
	};

	struct ElementInfo
	{
		SableString id;
		ElementType type = ElementType::Undef;

		LayoutProps layout;
		AppearanceProps appearance;
		TextProps text;
		Rect rect;

		std::function<void()> onHoverFunc = nullptr;
		std::function<void()> onHoverExitFunc = nullptr;
		std::function<void()> onClickFunc = nullptr;
		std::function<void()> onSecondaryClickFunc = nullptr;
		std::function<void()> onDoubleClickFunc = nullptr;
	};

	class BaseComponent;
	struct VirtualNode
	{
		VirtualNode();
		~VirtualNode();

		static int GetNumInstances();

		std::vector<VirtualNode*> children;
		ElementInfo info;
		BaseComponent* childComp = nullptr;
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
		Element(RendererBackend* renderer, ElementType type);
		~Element();

		static int GetNumInstances();

		ElementType type = ElementType::Undef;
		
		// functions for engine
		void Init(RendererBackend* renderer, ElementType type);
		void SetInfo(const ElementInfo& info);
		void SetRect(const Rect& rect);
		void AddChild(Element* child);
		void AddChild(Child* component);
		void SetImage(const std::string& path);
		void SetText(const SableString& text);
		int GetMinWidth();
		int GetMinHeight();

		ElementInfo info;
		ElementInfo GetInfo() const;

		// internal functions
		bool Reconcile(VirtualNode* vnode);
		void BuildRealSubtreeFromVirtual(VirtualNode* vnode);
		void BuildSingleElementFromVirtual(VirtualNode* vnode);

		// event system
		void el_PropagateEvents(const UIEventContext& ctx);
		bool el_PropagateComponentStateChanges(bool* hasContentsChanged = nullptr);
		Element* GetElementById(const SableString& id);

		// rendering
		void Render(int z = 1);
		bool clipEnabled = false;
		Rect clipRect = { 0, 0, 0, 0 };

		// children handling
		void LayoutChildren();
		bool layoutDirty = false;
		int measuredHeight = 0;
		std::vector<Child*> children;

	private:
		bool isHovered = false;
		DrawableBase* drawable = nullptr;
		RendererBackend* renderer = nullptr;
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