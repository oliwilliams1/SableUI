#include <SableUI/renderer/resource_handle.h>
#include <SableUI/utils/console.h>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "ResourceHandle"

namespace SableUI
{
    ResourceHandle ResourceHandleAllocator::Allocate(ResourceType type)
    {
        ResourceHandle handle;
        handle.type = type;

        if (!m_freeList.empty())
        {
            uint32_t index = m_freeList.back();
            m_freeList.pop_back();

            Slot& slot = m_slots[index];
            slot.generation++;
            slot.type = type;
            slot.active = true;

            handle.index = index;
            handle.generation = slot.generation;
        }
        else
        {
            uint32_t index = static_cast<uint32_t>(m_slots.size());

            Slot slot;
            slot.generation = 1;
            slot.type = type;
            slot.active = true;

            m_slots.push_back(slot);

            handle.index = index;
            handle.generation = 1;
        }

        return handle;
    }

    void ResourceHandleAllocator::Free(ResourceHandle handle)
    {
        if (!IsValid(handle))
        {
            SableUI_Warn("Attempting to free invalid handle (index: %u, gen: %u, type: %d)",
                handle.index, handle.generation, static_cast<int>(handle.type));
            return;
        }

        if (handle.index >= m_slots.size())
        {
            SableUI_Error("Handle index out of bounds: %u (size: %zu)",
                handle.index, m_slots.size());
            return;
        }

        Slot& slot = m_slots[handle.index];

        if (!slot.active)
        {
            SableUI_Warn("Attempting to free already freed handle (index: %u)", handle.index);
            return;
        }

        if (slot.generation != handle.generation)
        {
            SableUI_Error("Generation mismatch when freeing handle (expected: %u, got: %u)",
                slot.generation, handle.generation);
            return;
        }

        slot.active = false;
        slot.type = ResourceType::Invalid;
        m_freeList.push_back(handle.index);
    }

    bool ResourceHandleAllocator::IsValid(ResourceHandle handle) const
    {
        if (!handle.IsValid())
            return false;

        if (handle.index >= m_slots.size())
            return false;

        const Slot& slot = m_slots[handle.index];
        return slot.active &&
            slot.generation == handle.generation &&
            slot.type == handle.type;
    }
}