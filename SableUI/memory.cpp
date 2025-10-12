#include "SableUI/memory.h"
#include "SableUI/element.h"
#include "SableUI/component.h"
#include "SableUI/drawable.h"
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
static DynamicPool s_childPool;
static DynamicPool s_drawableRectPool;
static DynamicPool s_drawableImagePool;
static DynamicPool s_drawableTextPool;

static bool s_poolsInit = false;

void SableMemory::InitPools()
{
    if (s_poolsInit) return;

    Pool_Init(&s_elementPool, sizeof(SableUI::Element), 500);
    Pool_Init(&s_vnodePool, sizeof(SableUI::VirtualNode), 500);
    Pool_Init(&s_componentPool, sizeof(SableUI::BaseComponent), 100);
    Pool_Init(&s_childPool, sizeof(SableUI::Child), 600);
    Pool_Init(&s_drawableRectPool, sizeof(SableUI::DrawableRect), 250);
	Pool_Init(&s_drawableImagePool, sizeof(SableUI::DrawableImage), 50);
	Pool_Init(&s_drawableTextPool, sizeof(SableUI::DrawableText), 50);

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
    else if (size == sizeof(SableUI::Child))
    {
        return Pool_Alloc(&s_childPool);
    }
    else if (size == sizeof(SableUI::DrawableRect))
	{
		return Pool_Alloc(&s_drawableRectPool);
	}
	else if (size == sizeof(SableUI::DrawableImage))
	{
		return Pool_Alloc(&s_drawableImagePool);
	}
	else if (size == sizeof(SableUI::DrawableText))
	{
		return Pool_Alloc(&s_drawableTextPool);
	}

    return std::malloc(size);
}

void SableMemory::SB_free(void* ptr)
{
    if (!ptr) return;

    if (Pool_Free(&s_elementPool, ptr)) return;
    if (Pool_Free(&s_vnodePool, ptr)) return;
    if (Pool_Free(&s_componentPool, ptr)) return;
    if (Pool_Free(&s_childPool, ptr)) return;
    if (Pool_Free(&s_drawableRectPool, ptr)) return;
	if (Pool_Free(&s_drawableImagePool, ptr)) return;
	if (Pool_Free(&s_drawableTextPool, ptr)) return;

    std::free(ptr);
}

void SableMemory::DestroyPools()
{
    if (!s_poolsInit) return;

    Pool_Destroy(&s_elementPool);
    Pool_Destroy(&s_vnodePool);
    Pool_Destroy(&s_componentPool);
    Pool_Destroy(&s_childPool);
    Pool_Destroy(&s_drawableRectPool);
	Pool_Destroy(&s_drawableImagePool);
	Pool_Destroy(&s_drawableTextPool);

    s_poolsInit = false;
}