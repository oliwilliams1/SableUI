#pragma once
#include "SableUI/SableUI.h"
#include "SableUI/components/treeView.h"
#include "SableUI/element.h"
#include "SableUI/memory.h"

using namespace SableMemory;

namespace SableUI
{
	SableString GetPanelName(BasePanel* panel)
	{
		switch (panel->type)
		{
		case SableUI::PanelType::BASE:
			return "content panel";
			break;
		case SableUI::PanelType::HORIZONTAL:
			return "h-splitter";
			break;
		case SableUI::PanelType::VERTICAL:
			return "v-splitter";
			break;
		case SableUI::PanelType::UNDEF:
			return "undefined panel type";
			break;
		case SableUI::PanelType::ROOTNODE:
			return "root";
			break;
		default:
			return "unknown panel type";
			break;
		}
	}

	SableString ElementTypeToString(ElementType type)
	{
		switch (type)
		{
		case SableUI::ElementType::UNDEF:
			return "undefined element";
			break;
		case SableUI::ElementType::RECT:
			return "rect";
			break;
		case SableUI::ElementType::IMAGE:
			return "image";
			break;
		case SableUI::ElementType::TEXT:
			return "text";
			break;
		case SableUI::ElementType::DIV:
			return "div";
			break;
		case SableUI::ElementType::TEXT_U32:
			return "text u32";
			break;
		default:
			break;
		}
	}

	class DebugWindowView : public SableUI::BaseComponent
	{
	public:
		DebugWindowView(SableUI::Window* window) : SableUI::BaseComponent()
		{
			m_window = window;
			GenerateTreeView(m_window->GetRoot());
			m_window->AddUpdateCallback([this]() { WindowUpdate(m_window); });
		}

		~DebugWindowView() { if (m_window) m_window->ClearUpdateCallbacks(); }

		TreeNode BuildTreeFromPanel(BasePanel* panel)
		{
			if (panel == nullptr) return TreeNode("Null");

			TreeNode node(GetPanelName(panel));

			for (BasePanel* child : panel->children)
			{
				node.children.push_back(BuildTreeFromPanel(child));
			}

			if (Panel* contentPanel = dynamic_cast<Panel*>(panel))
			{
				node.children.push_back(BuildTreeFromElement(contentPanel->GetComponent()->GetRootElement()));
			}

			return node;
		}

		TreeNode BuildTreeFromElement(Element* element)
		{
			if (element == nullptr) return TreeNode("Null");

			TreeNode node(ElementTypeToString(element->type));

			for (Child* child : element->children)
			{
				Element* el = (Element*)*child;
				node.children.push_back(BuildTreeFromElement(el));
			}

			return node;
		}

		void GenerateTreeView(BasePanel* root)
		{
			rootNode = BuildTreeFromPanel(root);
		}

		void WindowUpdate(Window* window)
		{
			GenerateTreeView(window->GetRoot());
		}

		void Layout() override
		{
			Component(TreeView, bg(32, 32, 32), rootNode);
		}

	private:
		SableUI::Window* m_window = nullptr;
		TreeNode rootNode;
	};
}