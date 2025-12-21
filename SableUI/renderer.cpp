#include <SableUI/drawable.h>
#include <SableUI/renderer.h>
#include <SableUI/element.h> // For SB_delete (~Element())
#include <SableUI/console.h>
#include <SableUI/memory.h>
#include <SableUI/window.h>
#include <SableUI/utils.h>
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
	if (root)
		SableMemory::SB_delete(root);

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