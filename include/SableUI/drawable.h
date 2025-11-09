#pragma once
#include <vector>
#include "SableUI/texture.h"
#include "SableUI/text.h"
#include "SableUI/utils.h"

namespace SableUI
{
	enum class PanelType
	{
		ROOTNODE = 0x00,
		BASE = 0x01,
		VERTICAL = 0x02,
		HORIZONTAL = 0x03,
		UNDEF = 0xFF
	};

	struct GpuObject;
	struct ContextResources {
		GpuObject* rectObject = nullptr;
	};

	void InitDrawables();
	void DestroyDrawables();
	class RendererBackend;
	ContextResources& GetContextResources(RendererBackend* backend);

	struct RenderTarget;
	class DrawableBase
	{
	public:
		DrawableBase();
		virtual ~DrawableBase();
		static int GetNumInstances();
		virtual void Draw(RenderTarget* texture, ContextResources& res) = 0;
		void setZ(int z) { this->m_zIndex = z; }
		int m_zIndex = 0;
		Rect m_rect = { 0, 0, 0, 0 };
		unsigned int uuid = 0;
	private:
		unsigned int GetUUID();
	};

	class DrawableRect : public DrawableBase
	{
	public:
		DrawableRect();
		DrawableRect(Rect& r, Colour colour);
		~DrawableRect();
		static int GetNumInstances();
		void Update(Rect& rect, Colour colour, float pBSize = 0.0f);
		void Draw(RenderTarget* texture, ContextResources& res) override;
		Colour m_colour = { 255, 255, 255, 255 };
	};

	class DrawableSplitter : public DrawableBase
	{
	public:
		DrawableSplitter();
		DrawableSplitter(Rect& r, Colour colour);
		~DrawableSplitter();
		static int GetNumInstances();
		void Update(Rect& rect, Colour colour, PanelType type,
			float pBSize = 0.0f, const std::vector<int>& segments = { 0 });
		void Draw(RenderTarget* texture, ContextResources& res) override;
		Colour m_colour = { 255, 255, 255, 255 };
		int m_bSize = 0;
		std::vector<int> m_offsets;
		PanelType m_type = PanelType::UNDEF;
	};

	class DrawableImage : public DrawableBase
	{
	public:
		DrawableImage();
		~DrawableImage();
		static int GetNumInstances();
		void Update(Rect& rect) { this->m_rect = rect; }
		void Draw(RenderTarget* texture, ContextResources& res) override;
		Texture m_texture;
	};

	class DrawableText : public DrawableBase
	{
	public:
		DrawableText();
		~DrawableText();
		static int GetNumInstances();
		void Update(Rect& rect) { this->m_rect = rect; };
		void Draw(RenderTarget* texture, ContextResources& res) override;
		_Text m_text;
	};
}