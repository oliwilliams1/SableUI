#pragma once
#include <vector>
#include <cstdint>
#include <memory>

#include "SableUI/utils.h"
#include "SableUI/texture.h"
#include "SableUI/drawable.h"

namespace SableUI
{
	class Renderer
	{
	public:
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		static void Init(Texture* surface);
		static void Shutdown();
		static void SetSurface(Texture* surface);
		static void Flush();

		static Renderer& Get();

		void Draw(std::unique_ptr<SableUI::DrawableBase> drawable);

		void Draw();

	private:
		Renderer() {}

		std::vector<std::unique_ptr<SableUI::DrawableBase>> drawStack;
	};
}