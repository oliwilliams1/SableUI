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

		int SetContent(const std::u32string& str, int maxWidth, float fontSize = 12.0f, float lineSpacing = 1.25f);
		std::u32string m_content = U"";
		int m_fontSize = 0;
		float m_lineSpacing = 1.0f;
		
		GLuint m_fontTextureID = 0;
		GLuint m_VAO = 0;
		GLuint m_VBO = 0;
		GLuint m_EBO = 0;

		uint32_t indiciesSize = 0;
	};

	void InitFontManager();
	void DestroyFontManager();
}