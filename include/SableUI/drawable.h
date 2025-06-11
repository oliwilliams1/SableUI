#pragma once
#include <vector>

#include "SableUI/utils.h"
#include "SableUI/renderTarget.h"
#include "SableUI/texture.h"
#include "SableUI/text.h"

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

		void setZ(int z) { this->m_zIndex = z; }

		int m_zIndex = 0;
		SableUI::Rect m_rect = { 0, 0, 0, 0 };
	};

	class DrawableRect : public DrawableBase
	{
	public:
		DrawableRect() { this->m_zIndex = 0; };
		~DrawableRect() {};

		DrawableRect(SableUI::Rect& r, SableUI::Colour colour) 
			: m_colour(colour) { this->m_rect = r; this->m_zIndex = 0; }

		void Update(SableUI::Rect& rect, SableUI::Colour colour,
			float pBSize = 0.0f);

		void Draw(SableUI::RenderTarget* texture) override;

		SableUI::Colour m_colour = { 255, 255, 255, 255 };
	};

	class DrawableSplitter : public DrawableBase
	{
	public:
		DrawableSplitter() { this->m_zIndex = 1; };
		~DrawableSplitter() { m_offsets.clear(); };

		DrawableSplitter(SableUI::Rect& r, SableUI::Colour colour) 
			: m_colour(colour) { this->m_zIndex = 999; this->m_rect = r; }

		void Update(SableUI::Rect& rect, SableUI::Colour colour, SableUI::NodeType type, 
			float pBSize = 0.0f, const std::vector<int>& segments = { 0 });

		void Draw(SableUI::RenderTarget* texture) override;

		SableUI::Colour m_colour = { 255, 255, 255, 255 };
		int m_bSize = 0;
		std::vector<int> m_offsets;
		SableUI::NodeType m_type = SableUI::NodeType::UNDEF;
	};

	class DrawableImage : public DrawableBase
	{
	public:
		DrawableImage() { this->m_zIndex = 0; };
		~DrawableImage() {};

		void Update(SableUI::Rect& rect) { 
			this->m_rect = rect;
		}
		
		void Draw(SableUI::RenderTarget* texture) override;

		Texture m_texture;
	};

	class DrawableText : public DrawableBase
	{
	public:
		DrawableText() { this->m_zIndex = 0; };
		~DrawableText() {};

		void Update(SableUI::Rect& rect) { this->m_rect = rect; };

		void Draw(SableUI::RenderTarget* texture) override;

		Text m_text;
	};
}