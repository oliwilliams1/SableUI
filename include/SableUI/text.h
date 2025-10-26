#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <cstdint>
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
		std::string fontPath;

		bool operator<(const FontRange& other) const;
	};

	struct FontPack {
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
	
	enum class TextJustification {
		Left,
		Center,
		Right
	};

	struct TextCacheKey {
		uint64_t contentHash;
		int maxWidth;
		int fontSize;
		int maxHeight;
		int lineSpacingPx;
		TextJustification justify;

		bool operator==(const TextCacheKey& other) const;
	};

	struct _Text {
		_Text();
		~_Text();
		_Text(const _Text&) = delete;
		_Text& operator=(const _Text&) = delete;
		_Text(_Text&& other) noexcept;
		_Text& operator=(_Text&& other) noexcept;

		static int GetNumInstances();

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

	private:
		TextCacheKey m_cacheKey{};
		bool m_hasCachedBuffer = false;

		void ReleaseCache();
		TextCacheKey GenerateCacheKey() const;
	};

	struct TextCacheKeyHash {
		std::size_t operator()(const TextCacheKey& key) const;
	};

	struct CachedTextBuffer {
		CachedTextBuffer();

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		uint32_t indicesSize;
		int height;
		int refCount;
		void* context = nullptr;
		std::chrono::steady_clock::time_point lastUsed;
	};

	class TextCache {
	public:
		static TextCache& GetInstance();

		CachedTextBuffer* Acquire(const TextCacheKey& key, _Text* text);
		void Release(const TextCacheKey& key);
		void CleanupUnused(int secondsThreshold = 10);

		void Shutdown();
		static uint64_t HashString(const SableString& str);
		std::unordered_map<TextCacheKey, CachedTextBuffer, TextCacheKeyHash> m_cache;
	
	private:
		TextCache() = default;
		~TextCache() { Shutdown(); }
		TextCache(const TextCache&) = delete;
		TextCache& operator=(const TextCache&) = delete;
		CachedTextBuffer* CreateBuffer(const TextCacheKey& key, _Text* text);
		void DeleteBuffer(CachedTextBuffer& buffer);
	};

	GLuint GetAtlasTexture();
	void SetFontDPI(const vec2& dpi);
	void CleanupTextCache(int secondsThreshold);
	void InitFontManager();
	void DestroyFontManager();
}