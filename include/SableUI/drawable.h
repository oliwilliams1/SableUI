#pragma once
#include <vector>

#include "SableUI/utils.h"
#include "SableUI/renderTarget.h"
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
		virtual void Draw(SableUI::RenderTarget* texture) = 0;
		virtual ~DrawableBase() {};

		void setZ(int z) { this->z = z; }

		int z = 0;
		SableUI::Rect r = { 0, 0, 0, 0 };
	};

	class DrawableRect : public DrawableBase
	{
	public:
		DrawableRect() { this->z = 0; };
		~DrawableRect() {};

		DrawableRect(SableUI::Rect& r, SableUI::Colour colour) 
			: c(colour) { this->r = r; this->z = 0; }

		void Update(SableUI::Rect& rect, SableUI::Colour colour,
			float pBSize = 0.0f);

		void Draw(SableUI::RenderTarget* texture) override;

		SableUI::Colour c = { 255, 255, 255, 255 };
	};

	class DrawableSplitter : public DrawableBase
	{
	public:
		DrawableSplitter() { this->z = 1; };
		~DrawableSplitter() { offsets.clear(); };

		DrawableSplitter(SableUI::Rect& r, SableUI::Colour colour) 
			: c(colour) { this->z = 999; this->r = r; }

		void Update(SableUI::Rect& rect, SableUI::Colour colour, SableUI::NodeType type, 
			float pBSize = 0.0f, const std::vector<int>& segments = { 0 });

		void Draw(SableUI::RenderTarget* texture) override;

		SableUI::Colour c = { 255, 255, 255, 255 };
		int b = 0;
		std::vector<int> offsets;
		SableUI::NodeType type = SableUI::NodeType::UNDEF;
	};

	class DrawableImage : public DrawableBase
	{
	public:
		DrawableImage() { this->z = 0; };
		~DrawableImage() {};

		void Update(SableUI::Rect& rect) { 
			this->r = rect;
		}
		
		void Draw(SableUI::RenderTarget* texture) override;

		Texture t;
	};
}