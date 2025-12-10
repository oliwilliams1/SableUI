#include <cstdint>
#include <algorithm>
#include <SableUI/renderer.h>
#include <SableUI/memory.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h> // For SB_delete (~Element())
#include <SableUI/window.h>
#include <SableUI/utils.h>

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
SableUI::CustomTargetQueue::CustomTargetQueue(const GpuFramebuffer* target)
{
	this->target = target;
	s_targetQueueinstances++;
}

SableUI::CustomTargetQueue::~CustomTargetQueue()
{
	s_targetQueueinstances--;
	if (root)
		SableMemory::SB_delete(root);

	for (DrawableBase* dr : drawables)
		SableMemory::SB_delete(dr);

	if (queueContext)
		queueContext->RemoveQueueReference(this);
}

int SableUI::CustomTargetQueue::GetNumInstances()
{
	return s_targetQueueinstances;
}

void SableUI::CustomTargetQueue::AddRect(const Rect& rect, const Colour& colour, float borderRadius)
{
	DrawableRect* drRect = SableMemory::SB_new<DrawableRect>();
	drRect->Update(rect, colour, borderRadius, false, {});
	drawables.push_back(drRect);
}