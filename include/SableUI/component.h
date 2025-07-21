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

	class Body : public BaseComponent
	{
	public:
		Body(Node* parent = nullptr, Colour colour = Colour(255, 32, 32, 32))
			: BaseComponent(parent), colour(colour) {}
	
		void AddElement(Element* e);
		void UpdateElements();
		void Render() override;
		void UpdateDrawable(bool draw = true) override;
		void RenderElements();

		Element* m_element = nullptr;

	private:
		Colour colour;
		DrawableRect drawable;
	};

	class Splitter : public BaseComponent
	{
	public:
		Splitter(Node* parent = nullptr, Colour bColour = Colour(255, 32, 32, 32)) : BaseComponent(parent), bColour(bColour) {}

		void Render() override;
		void UpdateDrawable(bool draw = true) override;
	
	private:
		Colour bColour;
		DrawableSplitter drawable;
	};
}