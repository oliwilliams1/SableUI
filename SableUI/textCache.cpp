#include "SableUI/textCache.h"
#include "SableUI/renderer.h"

using namespace SableUI;

static std::unordered_map<RendererBackend*, TextCacheFactory> s_textCacheFactories;

GpuObject* TextCacheFactory::Get_priv(const _Text* text, int& height)
{
	TextCacheKey key = TextCacheKey(text);
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.refCount++;
		height = it->second.height;
		return it->second.gpuObject;
	}

	int actualLineWidth = 0;
	TextCache entry{};
	entry.gpuObject = GetTextGpuObject(text, height, actualLineWidth);
	entry.refCount++;
	entry.actualLineWidth = actualLineWidth;
	entry.height = height;
	entry.lastUsed = std::chrono::steady_clock::now();
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

	SableUI_Warn("Factory not found for renderer");
}

void SableUI::TextCacheFactory::ShutdownFactory(RendererBackend* renderer)
{
	auto it = s_textCacheFactories.find(renderer);
	if (it != s_textCacheFactories.end())
	{
		for (auto& pair : it->second.m_cache)
		{
			pair.second.gpuObject->m_context->DestroyGpuObject(pair.second.gpuObject);
		}
		s_textCacheFactories.erase(it);
	}
}

void TextCacheFactory::Release_priv(TextCacheKey key)
{
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.refCount--;
		if (it->second.refCount <= 0)
		{
			it->second.gpuObject->m_context->DestroyGpuObject(it->second.gpuObject);
			m_cache.erase(it);
		}
	}
	else
		SableUI_Warn("Entry not found");
}

void TextCacheFactory::Delete(TextCacheKey key)
{
	auto it = m_cache.find(key);
	if (it != m_cache.end())
	{
		it->second.gpuObject->m_context->DestroyGpuObject(it->second.gpuObject);
		m_cache.erase(it);
	}
	else
		SableUI_Warn("Called delete on a non-existent entry");
}

SableUI::TextCacheKey::TextCacheKey(const _Text* text)
{
	stringHash = std::hash<SableString>()(text->m_content);
	minWrapWidth = text->m_maxWidth;
	fontSize = text->m_fontSize;
	maxHeight = text->m_maxHeight;
	lineSpacingPx = text->m_lineSpacingPx;
	justification = text->m_justify;
}
