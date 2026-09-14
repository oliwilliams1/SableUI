#include <SableUI/core/text_cache.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/string.h>
#include <SableUI/core/text.h>
#include <SableUI/utils/utils.h>
#include <chrono>
#include <unordered_map>
#include <vector>

using namespace SableUI;

ResourceHandle TextCacheFactory::Get(CommandBuffer& cmd, const TextObj* text, int& height)
{
	TextCacheKey key = TextCacheKey(text);
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.refCount++;
		it->second.lastConsumedFrame = m_currentFrame;
		height = it->second.height;
		return it->second.gpuHandle;
	}

	int maxWidth = 0;
	TextCache entry{};
	entry.gpuHandle = GetTextGpuHandle(cmd, text, height, maxWidth);
	entry.refCount++;
	entry.maxWidth = text->m_maxWidth;
	entry.height = height;
	entry.lastConsumedFrame = m_currentFrame;
	m_cache[key] = entry;
	return entry.gpuHandle;
}

void TextCacheFactory::Release(TextCacheKey key)
{
	auto it = m_cache.find(key);

	if (it != m_cache.end())
		it->second.refCount--;
	else
		SableUI_Warn("Entry not found");
}

int SableUI::TextCacheFactory::GetNumInstances() const
{
	return m_cache.size();
}

void SableUI::TextCacheFactory::CleanCache(CommandBuffer& cmd)
{
	std::vector<TextCacheKey> toDelete;

	for (auto& pair : m_cache)
	{
		if (pair.second.refCount <= 0 && pair.second.lastConsumedFrame < m_currentFrame)
		{
			cmd.DestroyGpuObject(pair.second.gpuHandle);
			toDelete.push_back(pair.first);
		}
	}

	for (auto& key : toDelete)
		m_cache.erase(key);

	m_currentFrame++;
}

void TextCacheFactory::Delete(CommandBuffer& cmd, TextCacheKey key)
{
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		cmd.DestroyGpuObject(it->second.gpuHandle);
		m_cache.erase(it);
	}
	else
	{
		SableUI_Warn("Called delete on a non-existent entry");
	}
}

SableUI::TextCacheKey::TextCacheKey(const TextObj* text)
{
	stringHash = std::hash<SableString>()(text->m_content);
	maxWidth = text->m_maxWidth;
	fontSize = text->m_fontSize;
	maxHeight = text->m_maxHeight;
	lineSpacingPx = text->m_lineSpacingPx;
	justification = text->m_justify;
}
