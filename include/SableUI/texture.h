#pragma once

#include <GL/glew.h>
#include <string>

namespace SableUI
{
	void StepCachedTexturesCleaner();

	struct Texture
	{
		Texture();
		Texture(int width, int height, GLuint m_texID) : m_width(width), m_height(height), m_texID(m_texID) {}
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		static int GetNumInstances();
		void LoadTexture(const std::string& path);
		void LoadTextureOptimised(const std::string& path, int width = -1, int height = -1);
		void SetDefaultTexture(GLuint texID);
		void Bind() const;
		
		int m_width = -1;
		int m_height = -1;

	private:
		void SetTexture(uint8_t* pixels, int width, int height, int channels);
		GLuint m_texID = 0;

		void GenerateDefaultTexture();
		GLuint m_defaultTexID = 0;
	};
};