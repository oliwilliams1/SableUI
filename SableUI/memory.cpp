#include "SableUI/memory.h"
#include "SableUI/element.h"
#include "SableUI/component.h"
#include <cstdlib>
#include <vector>
#include <algorithm>

struct FreeNode {
    FreeNode* next = nullptr;
};

struct PoolChunk {
    void* buffer = nullptr;
    FreeNode* freeList = nullptr;
    size_t used = 0;
    size_t capacity = 0;
};

struct DynamicPool {
    size_t objectSize = 0;
    size_t initialCapacity = 0;
    std::vector<PoolChunk> chunks;
};

static void Pool_CreateChunk(DynamicPool* pool, size_t count);

static void Pool_Init(DynamicPool* pool, size_t objSize, size_t count)
{
    pool->objectSize = objSize;
    pool->initialCapacity = count;
    pool->chunks.reserve(4);
    Pool_CreateChunk(pool, count);
}

static void Pool_CreateChunk(DynamicPool* pool, size_t count)
{
    PoolChunk chunk;
    chunk.capacity = count;
    chunk.buffer = std::malloc(pool->objectSize * count);
    if (!chunk.buffer) return;

    chunk.freeList = nullptr;
    for (size_t i = 0; i < count; i++)
    {
        char* current = (char*)chunk.buffer + i * pool->objectSize;
        FreeNode* node = (FreeNode*)current;
        node->next = chunk.freeList;
        chunk.freeList = node;
    }

    chunk.used = 0;
    pool->chunks.push_back(chunk);
}

static void* Pool_Alloc(DynamicPool* pool)
{
    for (auto& chunk : pool->chunks)
    {
        if (chunk.freeList)
        {
            void* ptr = chunk.freeList;
            chunk.freeList = chunk.freeList->next;
            chunk.used++;
            return ptr;
        }
    }

    size_t growBy = std::max((size_t)1, (size_t)(pool->initialCapacity * 0.1));
    Pool_CreateChunk(pool, growBy);

    if (pool->chunks.empty() || !pool->chunks.back().buffer)
        return nullptr;

    return Pool_Alloc(pool);
}


static bool Pool_Free(DynamicPool* pool, void* ptr)
{
    for (auto it = pool->chunks.begin(); it != pool->chunks.end(); it++)
    {
        auto& chunk = *it;
        char* start = (char*)chunk.buffer;
        char* end = start + chunk.capacity * pool->objectSize;

        if (ptr >= start && ptr < end)
        {
            FreeNode* node = (FreeNode*)ptr;
            node->next = chunk.freeList;
            chunk.freeList = node;
            chunk.used--;

            if (chunk.used == 0 && pool->chunks.size() > 1)
            {
                std::free(chunk.buffer);
                pool->chunks.erase(it);
            }
            return true;
        }
    }
    return false; // not found
}

static void Pool_Destroy(DynamicPool* pool)
{
    for (auto& chunk : pool->chunks)
    {
        if (chunk.buffer)
        {
            std::free(chunk.buffer);
            chunk.buffer = nullptr;
        }
    }
    pool->chunks.clear();
}


static DynamicPool s_elementPool;
static DynamicPool s_vnodePool;
static DynamicPool s_componentPool;
static bool s_poolsInit = false;

void SableMemory::InitPools()
{
    if (s_poolsInit) return;

    Pool_Init(&s_elementPool, sizeof(SableUI::Element), 1000);
    Pool_Init(&s_vnodePool, sizeof(SableUI::VirtualNode), 1000);
    Pool_Init(&s_componentPool, sizeof(SableUI::BaseComponent), 100);

    s_poolsInit = true;
}

void* SableMemory::SB_alloc(size_t size)
{
    SableMemory::InitPools();

    if (size == sizeof(SableUI::Element))
    {
        return Pool_Alloc(&s_elementPool);
    }
    else if (size == sizeof(SableUI::VirtualNode))
    {
        return Pool_Alloc(&s_vnodePool);
    }
    else if (size == sizeof(SableUI::BaseComponent))
    {
        return Pool_Alloc(&s_componentPool);
    }

    return std::malloc(size);
}

void SableMemory::SB_free(void* ptr)
{
    if (!ptr) return;

    if (Pool_Free(&s_elementPool, ptr)) return;
    if (Pool_Free(&s_vnodePool, ptr)) return;
    if (Pool_Free(&s_componentPool, ptr)) return;

    std::free(ptr);
}

void SableMemory::DestroyPools()
{
    if (!s_poolsInit) return;

    Pool_Destroy(&s_elementPool);
    Pool_Destroy(&s_vnodePool);
    Pool_Destroy(&s_componentPool);

    s_poolsInit = false;
}