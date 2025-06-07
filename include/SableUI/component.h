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
		BaseComponent(Node* parent) : parentNode(parent) {};

		virtual void Render() = 0;
		virtual void UpdateDrawable(bool draw = true) = 0;
		Node* GetParent() { return parentNode; }
		void SetParent(SableUI::Node* n) { parentNode = n; }
		void SetRenderer(SableUI::Renderer* r) { renderer = r; }

	protected:
		Node* parentNode = nullptr;
		Renderer* renderer = nullptr;
	};

	class DefaultComponent : public BaseComponent
	{
	public:
		DefaultComponent(Colour colour = Colour(255, 32, 32, 32), 
			Node* parent = nullptr) : BaseComponent(parent), colour(colour) {}
	
		void AddElement(Element* e);
		void UpdateElements();
		void Render() override;
		void UpdateDrawable(bool draw = true) override;
		void RenderElements();

		std::vector<Element*> elements;

	private:
		Colour colour;
		DrawableRect drawable;
	};

	class SplitterComponent : public BaseComponent
	{
	public:
		SplitterComponent(Colour bColour = Colour(255, 51, 51, 51),
			Node* parent = nullptr) : BaseComponent(parent), bColour(bColour) {}

		void Render() override;
		void UpdateDrawable(bool draw = true) override;
	
	private:
		Colour bColour;
		DrawableSplitter drawable;
	};
}