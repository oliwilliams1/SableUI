#pragma once

#include <GL/glew.h>
#include <string>

namespace SableUI
{
	struct Texture
	{
		Texture() = default;
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		void LoadTexture(const std::string& path);
		void SetDefaultTexture(GLuint texID);
		void Bind() const;
		
		int m_width = -1;
		int m_height = -1;

	private:
		void SetTexture(uint8_t* pixels, int width, int height);
		GLuint m_texID = 0;

		uint8_t* GenerateDefaultTexture(int width, int height);
		GLuint m_defaultTexID = 0;
	};
};