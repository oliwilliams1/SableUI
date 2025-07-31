#pragma once
#include <vector>
#include <cstdint>
#include <memory>

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
		~Renderer() = default;

		void ClearStack();

		void Draw(DrawableBase* drawable);
		void Draw();

		void StartDirectDraw();
		void DirectDrawRect(const Rect& rect, const Colour& colour);
		void EndDirectDraw();

		RenderTarget renderTarget;

	private:
		bool directDraw = false;
		std::vector<DrawableBase*> drawStack;
	};
}