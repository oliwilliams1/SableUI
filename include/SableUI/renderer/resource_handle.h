#pragma once
#include <cstdint>
#include <vector>
#include <type_traits>

namespace SableUI
{
    enum class ResourceType : uint8_t
    {
        Invalid,
        GpuObject,
        Texture2D,
        Texture2DArray,
        UniformBuffer,
        Framebuffer
    };

    struct ResourceHandle
    {
        uint32_t index = 0;
        uint32_t generation = 0;
        ResourceType type = ResourceType::Invalid;

        bool IsValid() const { return type != ResourceType::Invalid; }
        bool operator==(const ResourceHandle& other) const {
            return index == other.index &&
                generation == other.generation &&
                type == other.type;
        }
    };

    class ResourceHandleAllocator
    {
    public:
        ResourceHandle Allocate(ResourceType type);
        void Free(ResourceHandle handle);
        bool IsValid(ResourceHandle handle) const;

    private:
        struct Slot
        {
            uint32_t generation = 0;
            ResourceType type = ResourceType::Invalid;
            bool active = false;
        };

        std::vector<Slot> m_slots;
        std::vector<uint32_t> m_freeList;
    };
}

namespace std
{
    template <>
    struct hash<SableUI::ResourceHandle>
    {
        size_t operator()(const SableUI::ResourceHandle& h) const noexcept
        {
            size_t h1 = hash<uint32_t>{}(h.index);
            size_t h2 = hash<uint32_t>{}(h.generation);
            size_t h3 = hash<uint8_t>{}(static_cast<uint8_t>(h.type));

            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}