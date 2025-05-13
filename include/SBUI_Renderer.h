#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include <memory>

#include "SBUI_Utils.h"

namespace Drawable
{
	class Base
	{
	public:
		virtual void Draw() {};
		virtual ~Base() {};
	};

	class Rect : public Base
	{
	public:
		Rect() {};
		~Rect() { rowBuffer.clear(); };
		Rect(SableUI::rect& r, SableUI::colour colour) : r(r), colour(colour) {}

		void Update(SableUI::rect& rect, SableUI::colour colour, float pBSize = 0.0f, bool draw = true);

		void Draw() override;

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour colour = { 255, 255, 255 };
		std::vector<uint32_t> rowBuffer;
	};

	class bSplitter : public Base
	{
	public:
		bSplitter() {};
		~bSplitter() { rowBuffer.clear(); colBuffer.clear(); };
		bSplitter(SableUI::rect& r, SableUI::colour colour) : r(r), colour(colour) {}

		void Update(SableUI::rect& rect, SableUI::colour colour, float pBSize = 0.0f, bool draw = true);

		void Draw() override;

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour colour = { 255, 255, 255 };
		std::vector<uint32_t> rowBuffer;
		std::vector<uint32_t> colBuffer;
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

		void Draw(std::unique_ptr<Drawable::Base> drawable);

		void Draw();

	private:
		Renderer() {}

		std::vector<std::shared_ptr<Drawable::Base>> drawStack;
	};
}