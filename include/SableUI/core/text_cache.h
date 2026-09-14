#pragma once
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/types/renderer_types.h>
#include <SableUI/core/text.h>
#include <unordered_map>
#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

namespace SableUI
{
	struct TextCacheKey
	{
		TextCacheKey(const TextObj* text);
		uint64_t stringHash;
		int maxWidth;
		int fontSize;
		int maxHeight;
		int lineSpacingPx;
		TextJustification justification;

		bool operator==(const TextCacheKey& other) const
		{
			return stringHash == other.stringHash &&
				maxWidth == other.maxWidth &&
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
			h ^= std::hash<int>()(key.maxWidth) + 0x9e3779b9 + (h << 6) + (h >> 2);
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
		ResourceHandle gpuHandle;
		int refCount;
		int maxWidth;
		int height;
		int lastConsumedFrame;

		bool operator==(const TextCache& other) const { return gpuHandle == other.gpuHandle; }
	};

	class TextCacheFactory
	{
	public:
		void CleanCache(CommandBuffer& cmd);
		ResourceHandle Get(CommandBuffer& cmd, const TextObj* key, int& height);
		void Release(TextCacheKey key);
		void Delete(CommandBuffer& cmd, TextCacheKey key);
		int GetNumInstances() const;

	private:
		int m_currentFrame = 0;
		std::unordered_map<TextCacheKey, TextCache> m_cache;
	};
}