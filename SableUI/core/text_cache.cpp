#include <SableUI/core/text_cache.h>

#include <chrono>
#include <unordered_map>
#include <vector>

#include <SableUI/core/renderer.h>
#include <SableUI/utils/console.h>
#include <SableUI/utils/string.h>
#include <SableUI/core/text.h>
#include <SableUI/utils/utils.h>

using namespace SableUI;

static std::unordered_map<RendererBackend*, TextCacheFactory> s_textCacheFactories;

GpuObject* TextCacheFactory::Get_priv(const _Text* text, int& height)
{
	TextCacheKey key = TextCacheKey(text);
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.refCount++;
		it->second.lastConsumedFrame = m_currentFrame;
		height = it->second.height;
		return it->second.gpuObject;
	}

	int maxWidth = 0;
	TextCache entry{};
	entry.gpuObject = GetTextGpuObject(text, height, maxWidth);
	entry.refCount++;
	entry.maxWidth = text->m_maxWidth;
	entry.height = height;
	entry.lastConsumedFrame = m_currentFrame;
	m_cache[key] = entry;
	return entry.gpuObject;
}

GpuObject* SableUI::TextCacheFactory::Get(const _Text* key, int& height)
{
	auto it = s_textCacheFactories.find(key->m_renderer);
	if (it != s_textCacheFactories.end())
		return it->second.Get_priv(key, height);

	s_textCacheFactories[key->m_renderer] = TextCacheFactory();
	return s_textCacheFactories[key->m_renderer].Get_priv(key, height);
}

void SableUI::TextCacheFactory::Release(RendererBackend* renderer, const TextCacheKey& key)
{
	auto it = s_textCacheFactories.find(renderer);
	if (it != s_textCacheFactories.end())
		return it->second.Release_priv(key);
}

void SableUI::TextCacheFactory::ShutdownFactory(RendererBackend* renderer)
{
	auto it = s_textCacheFactories.find(renderer);
	if (it != s_textCacheFactories.end())
	{
		for (auto& pair : it->second.m_cache)
		{
			pair.second.gpuObject->context->DestroyGpuObject(pair.second.gpuObject);
		}
		s_textCacheFactories.erase(it);
	}
}

std::vector<const TextCacheFactory*> SableUI::TextCacheFactory::GetFactories()
{
	std::vector<const TextCacheFactory*> factories;

	for (auto& pair : s_textCacheFactories)
		factories.push_back(&pair.second);

	return factories;
}

void TextCacheFactory::Release_priv(TextCacheKey key)
{
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.refCount--;
	}
	else
		SableUI_Warn("Entry not found");
}

int SableUI::TextCacheFactory::GetNumInstances() const
{
	return m_cache.size();
}

void SableUI::TextCacheFactory::CleanCache(RendererBackend* renderer)
{
	auto it = s_textCacheFactories.find(renderer);
	if (it != s_textCacheFactories.end())
		it->second.CleanCache_priv();
}

void SableUI::TextCacheFactory::CleanCache_priv()
{
	std::vector<TextCacheKey> toDelete;

	for (auto& pair : m_cache)
	{
		if (pair.second.refCount <= 0 && pair.second.lastConsumedFrame < m_currentFrame)
		{
			pair.second.gpuObject->context->DestroyGpuObject(pair.second.gpuObject);
			toDelete.push_back(pair.first);
		}
	}

	for (auto& key : toDelete)
		m_cache.erase(key);

	m_currentFrame++;
}

void TextCacheFactory::Delete(TextCacheKey key)
{
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.gpuObject->context->DestroyGpuObject(it->second.gpuObject);
		m_cache.erase(it);
	}
	else
		SableUI_Warn("Called delete on a non-existent entry");
}

SableUI::TextCacheKey::TextCacheKey(const _Text* text)
{
	stringHash = std::hash<SableString>()(text->m_content);
	maxWidth = text->m_maxWidth;
	fontSize = text->m_fontSize;
	maxHeight = text->m_maxHeight;
	lineSpacingPx = text->m_lineSpacingPx;
	justification = text->m_justify;
}
