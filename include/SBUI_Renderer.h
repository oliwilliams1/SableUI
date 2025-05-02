#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>

#include "SBUI_Utils.h"

struct SbUI_Rect
{
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t w = 0;
	uint16_t h = 0;
};

namespace Drawable
{
	struct SbUI_DrawableRect
	{
		SbUI_Rect rect;
		SbUIcolour colour;
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

	void DrawRect(const SbUI_Rect& rect, const SbUIcolour& colour);
	void Clear(const SbUIcolour& colour);

	void Draw();

private:
	SbUI_Renderer(SDL_Surface* surface) : surface(surface) {}
	std::vector<Drawable::SbUI_DrawableRect> queue;

	SDL_Surface* surface;
};