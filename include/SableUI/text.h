#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <chrono>

#include "SableUI/utils.h"

namespace SableUI
{
	struct FontRange {
		FontRange();
		FontRange(char32_t start, char32_t end, std::string fontPath);
		FontRange(const FontRange& other);
		~FontRange();
		static int GetNumInstances();

		char32_t start = 0;
		char32_t end = 0;
		std::string fontPath = "";

		bool operator<(const FontRange& other) const {
			if (start != other.start) return start < other.start;
			if (end != other.end) return end < other.end;
			return fontPath < other.fontPath;
		}
	};

	struct FontPack {
		FontPack();
		FontPack(const FontPack& other);
		FontPack(FontPack&& other) noexcept;
		FontPack& operator=(const FontPack& other);
		FontPack& operator=(FontPack&& other) noexcept;
		~FontPack();
		static int GetNumInstances();
		std::string fontPath = "";
		std::chrono::steady_clock::time_point lastConsumed;
		std::vector<FontRange> fontRanges;
	};
	
	enum class TextJustification {
		Left,
		Center,
		Right
	};

	GLuint GetAtlasTexture();
	void SetFontDPI(const vec2& dpi);
	struct _Text
	{
		_Text();
		~_Text();
		static int GetNumInstances();

		_Text(const _Text&) = delete;
		_Text& operator=(const _Text&) = delete;

		int SetContent(const SableString& str, int maxWidth, int fontSize = 11, int maxHeight = -1,
			float lineSpacing = 1.15f, TextJustification justification = TextJustification::Left);
		int UpdateMaxWidth(int maxWidth);
		int GetMinWidth();
		int GetUnwrappedHeight();
		SableString m_content;
		SableString m_actualContent;
		Colour m_colour = { 255, 255, 255, 255 };
		int m_fontSize = 0;
		int m_maxWidth = 0;
		int m_maxHeight = 0;
		int m_lineSpacingPx = 0;
		TextJustification m_justify = TextJustification::Left;

		GLuint m_fontTextureID = 0;
		GLuint m_VAO = 0;
		GLuint m_VBO = 0;
		GLuint m_EBO = 0;

		uint32_t indiciesSize = 0;
	};

	void InitFontManager();
	void DestroyFontManager();
}