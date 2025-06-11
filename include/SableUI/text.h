#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>

#include "SableUI/utils.h"

namespace SableUI
{
	struct CharDrawInfo {
		SableUI::vec2 relPos = { 0, 0 };
		SableUI::vec2 size = { 0, 0 };
		SableUI::vec4 uv = { 0, 0, 0, 0 };
		GLuint atlasTextureID = 0;
	};

	struct Text
	{
		Text() = default;
		~Text() {};

		Text(const Text&) = delete;
		Text& operator=(const Text&) = delete;

		void SetContent(const std::u32string& str);
		std::vector<CharDrawInfo> m_drawInfo;

	private:
		std::u32string m_content = U"";
	};

	void InitFontManager();
	void DestroyFontManager();
}