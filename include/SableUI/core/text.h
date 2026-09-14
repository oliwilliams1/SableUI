#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/command_buffer.h>
#include <string>
#include <vector>
#include <cstdint>

namespace SableUI
{
	struct FontRange
	{
		FontRange();
		FontRange(char32_t start, char32_t end, std::string fontPath);
		FontRange(const FontRange& other);
		~FontRange();
		static int GetNumInstances();

		char32_t start = 0;
		char32_t end = 0;
		std::string fontPath;

		bool operator<(const FontRange& other) const;
	};

	struct FontPack
	{
		FontPack();
		FontPack(const FontPack& other);
		FontPack(FontPack&& other) noexcept;
		FontPack& operator=(const FontPack& other);
		FontPack& operator=(FontPack&& other) noexcept;
		~FontPack();
		static int GetNumInstances();
		std::string fontPath;
		std::vector<FontRange> fontRanges;
	};
	
	enum class TextJustification
	{
		Left,
		Center,
		Right
	};

	struct TextSizeResult
	{
		int width;
		int height;
		int lineCount;
	};
	
	TextSizeResult QueryTextSize(
		SableUI::CommandBuffer& cmd,
		const SableString& text,
		int maxWidth,
		int fontSize = 11,
		float lineSpacing = 1.15f,
		TextJustification justification = TextJustification::Left,
		int maxHeight = -1
	);

	struct CursorPosition
	{
		int x;
		int y;
		int lineHeight;
		int lineIndex;
	};

	CursorPosition QueryCursorPosition(
		SableUI::CommandBuffer& cmd,
		const SableString& text,
		size_t cursorIndex,
		int maxWidth,
		int fontSize = 11,
		float lineSpacing = 1.15f,
		TextJustification justification = TextJustification::Left
	);

	class RendererBackend;
	struct TextCacheKey;
	struct TextObj {
		TextObj();
		~TextObj();
		TextObj(const TextObj&) = delete;
		TextObj& operator=(const TextObj&) = delete;
		TextObj(TextObj&& other) noexcept;
		TextObj& operator=(TextObj&& other) = delete;

		static int GetNumInstances();

		int SetContent(
			CommandBuffer& cmd,
			RendererBackend* renderer,
			const SableString& str,
			int maxWidth,
			int fontSize = 11,
			int maxHeight = -1,
			float lineSpacing = 1.15f,
			TextJustification justification = TextJustification::Left
		);

		int UpdateMaxWidth(CommandBuffer& cmd, int maxWidth);
		int GetMinWidth(CommandBuffer& cmd, bool wrapped);
		int GetUnwrappedHeight();

		SableString m_content;
		Colour m_colour = { 255, 255, 255, 255 };
		int m_fontSize = 0;
		int m_maxWidth = 0;
		int m_maxHeight = 0;
		int m_lineSpacingPx = 0;
		int m_cachedHeight = 0;
		int m_actualWrappedWidth = 0;
		TextJustification m_justify = TextJustification::Left;
		unsigned int m_fontTextureID = 0;
		ResourceHandle m_gpuObject;
		uint32_t indiciesSize = 0;
		RendererBackend* m_renderer = nullptr;

	private:
		std::vector<TextCacheKey> m_cacheKeys;
	};

	ResourceHandle GetTextGpuHandle(CommandBuffer& cmd, const TextObj* text, int& height, int& maxWidth);

	ResourceHandle GetTextAtlasTexture();
	void SetFontDPI(const vec2& dpi);
	void InitFontManager(SableUI::CommandBuffer& cmd);
	void DestroyFontManager();
}