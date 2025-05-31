#pragma once
#include <vector>

#include "SableUI/utils.h"
#include "SableUI/texture.h"

namespace SableUI
{
	class Renderer;

	enum class NodeType
	{
		ROOTNODE = 0x00,
		COMPONENT = 0x01,
		VSPLITTER = 0x02,
		HSPLITTER = 0x03,
		UNDEF = 0xFF
	};

	class DrawableBase
	{
	public:
		virtual void Draw(SableUI::Texture* texture) = 0;
		virtual ~DrawableBase() {};

		void setZ(int z) { this->z = z; }

		int z = 0;
		SableUI::rect r = { 0, 0, 0, 0 };
	};

	class DrawableRect : public DrawableBase
	{
	public:
		DrawableRect() { this->z = 0; };
		~DrawableRect() {};
		DrawableRect(SableUI::rect& r, SableUI::Colour colour) 
			: c(colour) { this->r = r; this->z = 0; }

		void Update(SableUI::rect& rect, SableUI::Colour colour,
			float pBSize = 0.0f);

		void Draw(SableUI::Texture* texture) override;

		SableUI::Colour c = { 255, 255, 255, 255 };
	};

	class DrawableSplitter : public DrawableBase
	{
	public:
		DrawableSplitter() { this->z = 1; };
		~DrawableSplitter() { offsets.clear(); };
		DrawableSplitter(SableUI::rect& r, SableUI::Colour colour) 
			: c(colour) { this->z = 999; this->r = r; }

		void Update(SableUI::rect& rect, SableUI::Colour colour, SableUI::NodeType type, 
			float pBSize = 0.0f, const std::vector<int>& segments = { 0 });

		void Draw(SableUI::Texture* texture) override;

		SableUI::Colour c = { 255, 255, 255, 255 };
		int b = 0;
		std::vector<int> offsets;
		SableUI::NodeType type = SableUI::NodeType::UNDEF;
	};
}