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
	class BaseElement;

	class Renderer
	{
	public:
		Renderer() {};
		~Renderer();

		void Flush();

		void Draw(std::unique_ptr<SableUI::DrawableBase> drawable);
		void Draw();

		BaseElement* CreateElement(const std::string& name);
		BaseElement* GetElement(const std::string& name);

		Texture texture;

	private:
		void DrawWindowBorder();

		std::vector<std::unique_ptr<SableUI::DrawableBase>> drawStack;
		std::vector<BaseElement*> elements;
	};
}