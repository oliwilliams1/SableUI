#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>

#include "SableUI/utils.h"

namespace SableUI
{
	GLuint GetAtlasTexture();
	struct Text
	{
		Text();
		~Text();

		Text(const Text&) = delete;
		Text& operator=(const Text&) = delete;

		void SetContent(const std::u32string& str, int fontSize = 12);
		std::u32string m_content = U"";
		int m_fontSize = 0;
		
		GLuint m_fontTextureID = 0;
		GLuint m_VAO = 0;
		GLuint m_VBO = 0;
		GLuint m_EBO = 0;

		uint32_t indiciesSize = 0;
	};

	void InitFontManager();
	void DestroyFontManager();
}