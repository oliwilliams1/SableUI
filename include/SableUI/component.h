#pragma once
#include "SableUI/node.h"
#include "SableUI/renderer.h"
#include "SableUI/element.h"
#include <unordered_map>

namespace SableUI
{
	struct Node;

	class BaseComponent
	{
	public:
		BaseComponent(SableUI::Node* parent) : parentNode(parent) {};

		virtual void Render() = 0;
		virtual void UpdateDrawable(bool draw = true) = 0;
		SableUI::Node* GetParent() { return parentNode; }
		void SetParent(SableUI::Node* n) { parentNode = n; }

	protected:
		SableUI::Node* parentNode = nullptr;
	};

	class DefaultComponent : public BaseComponent
	{
	public:
		DefaultComponent(SableUI::colour colour = SableUI::colour(255, 32, 32, 32), 
			SableUI::Node* parent = nullptr) : BaseComponent(parent), colour(colour) {}
	
		void AddElement(BaseElement* e);
		void UpdateElements();
		void Render() override;
		void UpdateDrawable(bool draw = true) override;
		void RenderElements();

		std::vector<BaseElement*> elements;

	private:
		SableUI::colour colour;
		SableUI::DrawableRect drawable;
	};

	class SplitterComponent : public BaseComponent
	{
	public:
		SplitterComponent(SableUI::colour bColour = SableUI::colour(255, 51, 51, 51),
			SableUI::Node* parent = nullptr) : BaseComponent(parent), bColour(bColour) {}

		void Render() override;
		void UpdateDrawable(bool draw = true) override;
	
	private:
		SableUI::colour bColour;
		SableUI::DrawableSplitter drawable;
	};
}