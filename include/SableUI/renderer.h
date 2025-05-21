#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include <memory>

#include "SableUI/utils.h"

enum class NodeType
{
	ROOTNODE = 0x00,
	COMPONENT = 0x01,
	VSPLITTER = 0x02,
	HSPLITTER = 0x03,
	UNDEF = 0xFF
};

namespace Drawable
{
	class Base
	{
	public:
		virtual void Draw() = 0;
		virtual ~Base() {};
	};

	class Rect : public Base
	{
	public:
		Rect() {};
		~Rect() {};
		Rect(SableUI::rect& r, SableUI::colour colour) : r(r), c(colour) {}

		void Update(SableUI::rect& rect, SableUI::colour colour,
			float pBSize = 0.0f, bool draw = true);

		void Draw() override;

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour c = { 255, 255, 255, 255 };
	};

	class bSplitter : public Base
	{
	public:
		bSplitter() {};
		~bSplitter() { offsets.clear(); };
		bSplitter(SableUI::rect& r, SableUI::colour colour) : r(r), c(colour) {}

		void Update(SableUI::rect& rect, SableUI::colour colour, NodeType type, float pBSize = 0.0f, 
			const std::vector<int>& segments = { 0 }, bool draw = true);

		void Draw() override;

		SableUI::rect r = { 0, 0, 0, 0 };
		SableUI::colour c = { 255, 255, 255, 255 };
		int b = 0;
		std::vector<int> offsets;
		NodeType type = NodeType::UNDEF;
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