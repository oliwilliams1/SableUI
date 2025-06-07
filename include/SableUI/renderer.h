#pragma once
#include <vector>
#include <cstdint>
#include <memory>

#include "SableUI/utils.h"
#include "SableUI/texture.h"
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

		Element* CreateElement(const std::string& name, ElementType type);
		Element* GetElement(const std::string& name);

		Texture texture;

	private:
		void DrawWindowBorder() const;

		std::vector<DrawableBase*> drawStack;
		std::vector<Element*> elements;
	};
}