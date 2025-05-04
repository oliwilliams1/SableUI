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

class SbUI_Renderer
{
public:
	SbUI_Renderer(const SbUI_Renderer&) = delete;
	SbUI_Renderer& operator=(const SbUI_Renderer&) = delete;

	static void Init(SDL_Surface* surface);
	static void Shutdown();

	static SbUI_Renderer& Get();

	void DrawRect(const SableUI::rect& rect, const SableUI::colour& colour);
	void Clear(const SableUI::colour& colour);

	void Draw();

private:
	SbUI_Renderer(SDL_Surface* surface) : surface(surface) {}
	std::vector<Drawable::SbUI_DrawableRect> queue;

	SDL_Surface* surface;
};