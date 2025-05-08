#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>

#include "SBUI_Utils.h"

namespace Drawable
{
	struct rect
	{
		rect() {};
		rect(SableUI::rect& r, SableUI::colour colour) : r(r), colour(colour) {}
		
		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour colour = { 255, 255, 255 };
	};

	struct rectBorder
	{
		rectBorder() {};
		rectBorder(SableUI::rect& r, SableUI::colour colour, float border) : r(r), colour(colour), border(border) {}

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour colour = { 255, 255, 255 };
		float border = 0.0f;
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

		Drawable::rect GetDrawableRect(const SableUI::rect& rect, const SableUI::colour& colour, float border = 0.0f, bool draw = true);
		void Clear(const SableUI::colour& colour);

		void Draw();

		std::vector<Drawable::rect> rectQueue;
		std::vector<Drawable::rectBorder> rectBorderQueue;

	private:
		Renderer(SDL_Surface* surface) : surface(surface) {}

		void DrawRects(int surfaceWidth, int surfaceHeight);

		void DrawRectBorders();

		SDL_Surface* surface;
	};
}