#include <cstdint>
#include <algorithm>
#include <SableUI/renderer.h>
#include <SableUI/memory.h>
#include <SableUI/drawable.h>
#include <SableUI/element.h> // For SB_delete (~Element())
#include <SableUI/window.h>
#include <SableUI/utils.h>
#include <SableUI/console.h>

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
	drRect->Update(rect, colour, borderRadius);
	drawables.push_back(drRect);
}

void SableUI::RendererBackend::PushScissor(int x, int y, int width, int height)
{
    Rect newRect = { x, y, width, height };

    if (!m_scissorStack.empty())
    {
        const Rect& parentRect = m_scissorStack.back();

        int newX = std::max(parentRect.x, x);
        int newY = std::max(parentRect.y, y);
        int newRight = std::min(parentRect.x + parentRect.width, x + width);
        int newBottom = std::min(parentRect.y + parentRect.height, y + height);

        newRect.x = newX;
        newRect.y = newY;
        newRect.width = std::max(0, newRight - newX);
        newRect.height = std::max(0, newBottom - newY);
    }

    m_scissorStack.push_back(newRect);
}

void SableUI::RendererBackend::PopScissor()
{
    if (!m_scissorStack.empty())
    {
        m_scissorStack.pop_back();
    }
    else
    {
        SableUI_Error("Attempted to pop empty scissor stack");
    }
}