#include <SableUI/core/drawable.h>
#include <SableUI/renderer/renderer.h>
#include <SableUI/core/element.h> // For SB_delete (~Element())
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/window.h>
#include <SableUI/utils/utils.h>
#include <cstdint>
#include <optional>

uint32_t SableUI::RendererBackend::AllocateHandle()
{
	if (!m_freeHandles.empty())
	{
		uint32_t handle = m_freeHandles.back();
		m_freeHandles.pop_back();
		return handle;
	}
	return m_nextHandle++;
}

void SableUI::RendererBackend::FreeHandle(uint32_t handle)
{
	m_freeHandles.push_back(handle);
}