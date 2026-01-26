#pragma once
#include <SableUI/core/texture.h>
#include <SableUI/core/text.h>
#include <SableUI/utils/utils.h>
#include <SableUI/core/shader.h>
#include <vector>
#include <cstdint>
#include <optional>

namespace SableUI
{
	enum class PanelType
	{
		Root,
		Base,
		VerticalSplitter,
		HorizontalSplitter,
		Floating,
		Undef
	};

	struct GpuObject;
	struct GpuFramebuffer;
	struct ContextResources {
		GpuObject* rectObject = nullptr;
	};

	struct GlobalResources {
		Shader s_rect;
		uint32_t u_rectColour = 0;
		uint32_t u_rectRect = 0;
		uint32_t u_rectTexBool = 0;
		uint32_t u_rectRealRect = 0;
		uint32_t u_rectRadius = 0;
		uint32_t u_rectBorderSize = 0;
		uint32_t u_rectBorderColour = 0;

		Shader s_text;
		uint32_t u_textTargetSize = 0;
		uint32_t u_textPos = 0;
		uint32_t u_textAtlas = 0;
	};

	void InitDrawables();
	void DestroyDrawables();
	class RendererBackend;
	ContextResources& GetContextResources(RendererBackend* backend);
	GlobalResources& GetGlobalResources();

	class DrawableBase
	{
	public:
		DrawableBase();
		virtual ~DrawableBase();
		static int GetNumInstances();
		virtual void Draw(const GpuFramebuffer* framebuffer, ContextResources& res) = 0;
		void setZ(int z) { this->m_zIndex = z; }
		int m_zIndex = 0;
		Rect m_rect = { 0, 0, 0, 0 };
		float m_rTL = 0.0f, m_rTR = 0.0f, m_rBL = 0.0f, m_rBR = 0.0f;
		int m_bT = 0, m_bB = 0, m_bL = 0, m_bR = 0;
		unsigned int uuid = 0;
		bool orphan = false;
		bool scissorEnabled = false;
		Rect scissorRect = { 0, 0, 0, 0 };

	private:
		unsigned int GetUUID();
	};

	class DrawableRect : public DrawableBase
	{
	public:
		DrawableRect();
		~DrawableRect();
		static int GetNumInstances();
		void Update(
			const Rect& rect, 
			std::optional<Colour> colour,
			float rTL, float rTR,
			float rBL, float rBR,
			std::optional<Colour> borderColour,
			int bT, int bB,
			int bL, int bR,
			bool clipEnabled,
			const Rect& clipRect);

		void Draw(const GpuFramebuffer* framebuffer, ContextResources& res) override;

		std::optional<Colour> m_colour = std::nullopt;
		std::optional<Colour> m_borderColour = std::nullopt;
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
		void Draw(const GpuFramebuffer* framebuffer, ContextResources& res) override;
		Colour m_colour = { 255, 255, 255, 255 };
		int m_bSize = 2;
		std::vector<int> m_offsets;
		PanelType m_type = PanelType::Undef;
	};

	class DrawableImage : public DrawableBase
	{
	public:
		DrawableImage();
		~DrawableImage();
		static int GetNumInstances();
		void Update(
			Rect& rect,
			float rTL, float rTR,
			float rBL, float rBR,
			std::optional<Colour> borderColour,
			int bT, int bB, 
			int bL, int bR,
			bool clipEnabled,
			const Rect& clipRect);

		void Draw(const GpuFramebuffer* framebuffer, ContextResources& res) override;
		void RegisterTextureDependancy(BaseComponent* component);
		void DeregisterTextureDependancy(BaseComponent* component);

		Texture m_texture;
		std::optional<Colour> m_borderColour = std::nullopt;
	};

	class DrawableText : public DrawableBase
	{
	public:
		DrawableText();
		~DrawableText();
		static int GetNumInstances();
		void Update(Rect& rect, bool clipEnabled,
			const Rect& clipRect);
		void Draw(const GpuFramebuffer* framebuffer, ContextResources& res) override;
		_Text m_text;
	};
}