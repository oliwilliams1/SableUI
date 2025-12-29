#pragma once
#include <vector>
#include <string>
#include <functional>

#include <SableUI/core/renderer.h>
#include <SableUI/core/events.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/text.h>
#include <SableUI/utils/utils.h>
#include <optional>

namespace SableUI
{
	enum class ElementType
	{
		Undef,
		Rect,
		Image,
		Text,
		Div
	};

	enum class LayoutDirection
	{
		UpDown,
		DownUp,
		LeftRight,
		RightLeft
	};

	enum class ComponentSize
	{
		Small,
		Medium,
		Large
	};

	struct LayoutProps {
		RectType wType = RectType::Undef;
		RectType hType = RectType::Undef;
		LayoutDirection layoutDirection = LayoutDirection::UpDown;
		int width = 0, height = 0;
		int minW = 0, maxW = 0, minH = 0, maxH = 0;
		int pT = 0, pB = 0, pL = 0, pR = 0;
		int mT = 0, mB = 0, mL = 0, mR = 0;
		bool centerX = false, centerY = false;
		ivec2 pos = { -1, -1 };
	};

	struct AppearanceProps {
		Colour bg = { 0, 0, 0, 0 };
		Colour hoverBg = { 0, 0, 0, 0 };
		bool hasHoverBg = false;
		bool inheritBg = true;
		float radius = 0.0f;

		ComponentSize size = ComponentSize::Medium; // scaling for sableui components
		bool disabled = false; // special property for sableui components
		bool clipChildren = false;
	};

	struct TextProps {
		SableString content;
		std::optional<Colour> colour = std::nullopt;
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
		Element() = delete;
		Element(RendererBackend* renderer, const ElementInfo& type);
		~Element();

		static int GetNumInstances();

		// functions for engine
		void Init(RendererBackend* renderer);
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
		Rect rect = { 0, 0, 0, 0 };
		bool clipEnabled = false;
		Rect clipRect = { 0, 0, 0, 0 };

		// children handling
		void LayoutChildren();
		bool layoutDirty = false;
		int measuredHeight = 0;
		std::vector<Child*> children;

		// logic for hover styling
		void RegisterForHover();
		BaseComponent* m_owner = nullptr;
		bool isHovered = false;
		bool wasHovered = false;
		Colour originalBg = { 0, 0, 0, 0 };

	private:
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