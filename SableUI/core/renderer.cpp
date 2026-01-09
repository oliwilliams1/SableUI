#include <SableUI/core/drawable.h>
#include <SableUI/core/renderer.h>
#include <SableUI/core/element.h> // For SB_delete (~Element())
#include <SableUI/utils/console.h>
#include <SableUI/utils/memory.h>
#include <SableUI/core/window.h>
#include <SableUI/utils/utils.h>
#include <cstdint>

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

static int s_targetQueueinstances = 0;
SableUI::CustomTargetQueue::CustomTargetQueue()
{
	s_targetQueueinstances++;
}

SableUI::CustomTargetQueue::~CustomTargetQueue()
{
	s_targetQueueinstances--;

	for (DrawableBase* dr : drawables)
		SableMemory::SB_delete(dr);

	if (window)
		window->RemoveQueueReference(this);
}

int SableUI::CustomTargetQueue::GetNumInstances()
{
	return s_targetQueueinstances;
}

void SableUI::CustomTargetQueue::AddRect(const Rect& rect, const Colour& colour, float borderRadius)
{
	if (!target)
		SableUI_Runtime_Error("CustomTargetQueue has not been initialised");

	DrawableRect* drRect = SableMemory::SB_new<DrawableRect>();
	drRect->Update(rect, colour, borderRadius, false, {});
	drawables.push_back(drRect);
}