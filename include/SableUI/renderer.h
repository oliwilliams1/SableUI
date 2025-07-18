#pragma once
#include <vector>
#include <cstdint>
#include <memory>

#include "SableUI/renderTarget.h"
#include "SableUI/drawable.h"
#include "SableUI/element.h"

namespace SableUI
{
	class Element;

	enum class ElementType;

	class Renderer
	{
	public:
		Renderer() {};
		~Renderer();

		void Flush();

		void Draw(DrawableBase* drawable);
		void Draw();

		void StartDirectDraw();
		void DirectDrawRect(const Rect& rect, const Colour& colour);
		void EndDirectDraw();

		Element* CreateElement(const std::string& name, ElementType type);
		Element* GetElement(const std::string& name);

		RenderTarget renderTarget;

	private:
		bool directDraw = false;
		std::vector<DrawableBase*> drawStack;
		std::vector<Element*> elements;
	};
}