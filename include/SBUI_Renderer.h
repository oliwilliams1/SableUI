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
		Rect(SableUI::rect& r, SableUI::colour colour, float border = 0.0f) 
			: r(r), colour(colour), border(border) {}

		void Update(SableUI::rect& rect, SableUI::colour colour, float border = 0.0f, 
			SableUI::colour bColour = { 32, 32, 32 }, bool draw = true);

		void Draw() override;

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour colour = { 255, 255, 255 };
		std::vector<uint32_t> rowBuffer;

		float border = 0.0f;
		SableUI::colour bColour = { 255, 255, 255 };
		std::vector<uint32_t> bTBRowBuffer;
		std::vector<uint32_t> bLRRowBuffer;
	};

	enum class SizeType
	{
		PX = 0,
		PERCENT = 1,
		AUTO = 2
	};

	struct size
	{
		size() {};
		size(const char* str);

		SizeType type = SizeType::AUTO;

		float v = -1.0f;
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

		void Clear(const SableUI::colour& colour);

		void Draw(std::unique_ptr<Drawable::Base> drawable);

		void Draw();

	private:
		Renderer() {}

		std::vector<std::shared_ptr<Drawable::Base>> drawStack;
	};
}