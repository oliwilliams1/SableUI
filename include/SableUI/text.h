#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>

#include "SableUI/utils.h"

namespace SableUI
{
	GLuint GetAtlasTexture();
	void SetFontDPI(const vec2& dpi);
	struct Text
	{
		Text();
		~Text();

		Text(const Text&) = delete;
		Text& operator=(const Text&) = delete;

		int SetContent(const SableString& str, int maxWidth, int fontSize = 11, float lineSpacing = 1.15f);
		int UpdateMaxWidth(int maxWidth);
		int GetUnwrappedWidth();
		int GetUnwrappedHeight();
		SableString m_content = U"";
		SableString m_actualContent = U"";
		int m_fontSize = 0;
		int m_maxWidth = 0;
		int m_lineSpacingPx = 0;
		
		GLuint m_fontTextureID = 0;
		GLuint m_VAO = 0;
		GLuint m_VBO = 0;
		GLuint m_EBO = 0;

		uint32_t indiciesSize = 0;
	};

	void InitFontManager();
	void DestroyFontManager();
}