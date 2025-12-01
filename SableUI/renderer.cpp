#include <SableUI/renderer.h>
#include <SableUI/memory.h>
#include <SableUI/element.h> // for SB_delete, (~Element())
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
}

int SableUI::CustomTargetQueue::GetNumInstances()
{
	return s_targetQueueinstances;
}