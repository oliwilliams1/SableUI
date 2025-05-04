#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>

#include "SBUI_Utils.h"

namespace SableUI
{
	struct rect
	{
		float x = 0;
		float y = 0;
		float w = 0;
		float h = 0;
	};
}

namespace Drawable
{
	struct SbUI_DrawableRect
	{
		SableUI::rect rect;
		SableUI::colour colour;
	};
}

namespace SableUI
{
	class Renderer
	{
	public:
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		static void Init(SDL_Surface* surface);
		static void Shutdown();
		static void SetSurface(SDL_Surface* surface);

		static Renderer& Get();

		void DrawRect(const SableUI::rect& rect, const SableUI::colour& colour);
		void Clear(const SableUI::colour& colour);

		void Draw();

	private:
		Renderer(SDL_Surface* surface) : surface(surface) {}
		std::vector<Drawable::SbUI_DrawableRect> queue;

		SDL_Surface* surface;
	};
}