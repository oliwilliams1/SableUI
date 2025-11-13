#pragma once
#include <unordered_map>
#include <chrono>
#include "SableUI/text.h"

namespace SableUI
{
	struct _Text;
	struct GpuObject;
	enum class TextJustification;
	class RendererBackend;
	struct TextCacheKey
	{
		TextCacheKey(const _Text* text);
		uint64_t stringHash;
		int minWrapWidth;
		int fontSize;
		int maxHeight;
		int lineSpacingPx;
		TextJustification justification;

		bool operator==(const TextCacheKey& other) const
		{
			return stringHash == other.stringHash &&
				minWrapWidth == other.minWrapWidth &&
				fontSize == other.fontSize &&
				maxHeight == other.maxHeight &&
				lineSpacingPx == other.lineSpacingPx &&
				justification == other.justification;
		}
	};
}

namespace std
{
	template<>
	struct hash<SableUI::TextCacheKey>
	{
		std::size_t operator()(const SableUI::TextCacheKey& key) const noexcept
		{
			std::size_t h = 0;

			h ^= std::hash<uint64_t>()(key.stringHash) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(key.minWrapWidth) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(key.fontSize) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(key.maxHeight) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(key.lineSpacingPx) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<int>()(static_cast<int>(key.justification)) + 0x9e3779b9 + (h << 6) + (h >> 2);

			return h;
		}
	};
}

namespace SableUI
{
	struct TextCache
	{
		GpuObject* gpuObject;
		int refCount;
		int actualLineWidth;
		int height;
		std::chrono::steady_clock::time_point lastUsed;

		bool operator==(const TextCache& other) const { return gpuObject == other.gpuObject; }
	};

	class TextCacheFactory
	{
	public:
		static GpuObject* Get(const _Text* key, int& height);
		static void Release(RendererBackend* renderer, const TextCacheKey& key);
		static void ShutdownFactory(RendererBackend* renderer);

	private:
		GpuObject* Get_priv(const _Text* key, int& height);
		void Release_priv(TextCacheKey key);

		void Delete(TextCacheKey key);
		std::unordered_map<TextCacheKey, TextCache> m_cache;
	};
}