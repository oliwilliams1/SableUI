#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include <memory>

#include "SableUI/utils.h"

namespace SableUI
{
	enum class NodeType
	{
		ROOTNODE = 0x00,
		COMPONENT = 0x01,
		VSPLITTER = 0x02,
		HSPLITTER = 0x03,
		UNDEF = 0xFF
	};
}

namespace SableUI_Drawable
{
	class Base
	{
	public:
		virtual void Draw() = 0;
		virtual ~Base() {};

		void setZ(int z) { this->z = z; }

		int z = 0;
		SableUI::rect r = { 0, 0, 0, 0 };
	};

	class Rect : public Base
	{
	public:
		Rect() { this->z = 0; };
		~Rect() { this->z = 0; };
		Rect(SableUI::rect& r, SableUI::colour colour) : c(colour) { this->r = r; this->z = 0; }

		void Update(SableUI::rect& rect, SableUI::colour colour,
			float pBSize = 0.0f, bool draw = true);

		void Draw() override;

		SableUI::colour c = { 255, 255, 255, 255 };
	};

	class bSplitter : public Base
	{
	public:
		bSplitter() { this->z = 1; };
		~bSplitter() { offsets.clear(); this->z = 1; };
		bSplitter(SableUI::rect& r, SableUI::colour colour) : c(colour) { this->z = 999; this->r = r; }

		void Update(SableUI::rect& rect, SableUI::colour colour, SableUI::NodeType type, float pBSize = 0.0f, 
			const std::vector<int>& segments = { 0 }, bool draw = true);

		void Draw() override;

		SableUI::colour c = { 255, 255, 255, 255 };
		int b = 0;
		std::vector<int> offsets;
		SableUI::NodeType type = SableUI::NodeType::UNDEF;
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
		static void Flush();

		static Renderer& Get();

		void Draw(std::unique_ptr<SableUI_Drawable::Base> drawable);

		void Draw(SDL_Window* window);

	private:
		Renderer() {}

		std::vector<std::unique_ptr<SableUI_Drawable::Base>> drawStack;
	};
}