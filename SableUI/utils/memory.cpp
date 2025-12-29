#include <SableUI/utils/memory.h>
#include <SableUI/core/element.h>
#include <SableUI/core/component.h>
#include <SableUI/core/drawable.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <malloc.h>
#include <cstdio>

struct FreeNode {
	FreeNode* next = nullptr;
};

struct PoolChunk {
	void* buffer = nullptr;
	FreeNode* freeList = nullptr;
	size_t used = 0;
	size_t capacity = 0;
	size_t maxUsed = 0;
};

struct DynamicPool {
	size_t objectSize = 0;
	size_t chunkSize = 0;
	std::vector<PoolChunk> chunks;

	size_t totalAllocations = 0;
	size_t totalFrees = 0;
	size_t peakUsage = 0;
};

static void Pool_Init(DynamicPool* pool, size_t objSize, size_t chunkSize)
{
	pool->objectSize = objSize;
	pool->chunkSize = chunkSize;
	pool->chunks.reserve(4);
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
	chunk.maxUsed = 0;
	pool->chunks.push_back(chunk);
}

static void* Pool_Alloc(DynamicPool* pool)
{
	if (pool->chunks.empty())
	{
		Pool_CreateChunk(pool, pool->chunkSize);
	}

	for (auto& chunk : pool->chunks)
	{
		if (chunk.freeList)
		{
			void* ptr = chunk.freeList;
			chunk.freeList = chunk.freeList->next;
			chunk.used++;
			chunk.maxUsed = std::max(chunk.maxUsed, chunk.used);

			pool->totalAllocations++;

			size_t currentTotal = 0;
			for (const auto& c : pool->chunks)
				currentTotal += c.used;
			pool->peakUsage = std::max(pool->peakUsage, currentTotal);

			return ptr;
		}
	}

	Pool_CreateChunk(pool, pool->chunkSize);

	if (pool->chunks.empty() || !pool->chunks.back().buffer)
		return nullptr;

	return Pool_Alloc(pool);
}

static bool Pool_Free(DynamicPool* pool, void* ptr)
{
	if (!ptr) return false;

	for (auto it = pool->chunks.begin(); it != pool->chunks.end(); it++)
	{
		auto& chunk = *it;
		char* start = (char*)chunk.buffer;
		char* end = start + chunk.capacity * pool->objectSize;

		if (ptr >= start && ptr < end)
		{
			std::memset(ptr, 0, pool->objectSize);

			FreeNode* node = (FreeNode*)ptr;
			node->next = chunk.freeList;
			chunk.freeList = node;
			chunk.used--;
			pool->totalFrees++;

			if (chunk.used == 0 && pool->chunks.size() > 1)
			{
				std::free(chunk.buffer);
				pool->chunks.erase(it);
				return true;
			}

			return true;
		}
	}
	return false;
}

static void Pool_Compact(DynamicPool* pool)
{
	if (pool->chunks.size() <= 1) return;

	size_t totalUsed = 0;
	size_t totalCapacity = 0;
	for (const auto& chunk : pool->chunks)
	{
		totalUsed += chunk.used;
		totalCapacity += chunk.capacity;
	}

	if (totalCapacity == 0) return;

	double utilization = (double)totalUsed / totalCapacity;

	if (utilization < 0.3 && pool->chunks.size() > 1)
	{
		auto it = pool->chunks.begin();
		while (it != pool->chunks.end() && pool->chunks.size() > 1)
		{
			if (it->used == 0)
			{
				std::free(it->buffer);
				it = pool->chunks.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
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
	pool->totalAllocations = 0;
	pool->totalFrees = 0;
	pool->peakUsage = 0;
}

static DynamicPool s_elementPool;
static DynamicPool s_vnodePool;
static DynamicPool s_componentPool;
static DynamicPool s_childPool;
static DynamicPool s_drawableRectPool;
static DynamicPool s_drawableImagePool;
static DynamicPool s_drawableTextPool;
static DynamicPool s_drawableSplitterPool;
static DynamicPool s_gpuObjectPool;

static bool s_poolsInit = false;
static size_t s_frameCount = 0;

void SableMemory::InitPools()
{
	if (s_poolsInit) return;

	Pool_Init(&s_elementPool, sizeof(SableUI::Element), 128);
	Pool_Init(&s_vnodePool, sizeof(SableUI::VirtualNode), 128);
	Pool_Init(&s_componentPool, sizeof(SableUI::BaseComponent), 32);
	Pool_Init(&s_childPool, sizeof(SableUI::Child), 128);
	Pool_Init(&s_drawableRectPool, sizeof(SableUI::DrawableRect), 64);
	Pool_Init(&s_drawableImagePool, sizeof(SableUI::DrawableImage), 32);
	Pool_Init(&s_drawableTextPool, sizeof(SableUI::DrawableText), 32);
	Pool_Init(&s_drawableSplitterPool, sizeof(SableUI::DrawableSplitter), 16);
	Pool_Init(&s_gpuObjectPool, sizeof(SableUI::GpuObject), 32);

	s_poolsInit = true;
}

void* SableMemory::SB_alloc(size_t size)
{
	InitPools();

	if (size == sizeof(SableUI::Element))
		return Pool_Alloc(&s_elementPool);
	else if (size == sizeof(SableUI::VirtualNode))
		return Pool_Alloc(&s_vnodePool);
	else if (size == sizeof(SableUI::BaseComponent))
		return Pool_Alloc(&s_componentPool);
	else if (size == sizeof(SableUI::Child))
		return Pool_Alloc(&s_childPool);
	else if (size == sizeof(SableUI::DrawableRect))
		return Pool_Alloc(&s_drawableRectPool);
	else if (size == sizeof(SableUI::DrawableImage))
		return Pool_Alloc(&s_drawableImagePool);
	else if (size == sizeof(SableUI::DrawableText))
		return Pool_Alloc(&s_drawableTextPool);
	else if (size == sizeof(SableUI::DrawableSplitter))
		return Pool_Alloc(&s_drawableSplitterPool);
	else if (size == sizeof(SableUI::GpuObject))
		return Pool_Alloc(&s_gpuObjectPool);

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
	if (Pool_Free(&s_drawableSplitterPool, ptr)) return;
	if (Pool_Free(&s_gpuObjectPool, ptr)) return;

	std::free(ptr);
}

void SableMemory::CompactPools()
{
	if (!s_poolsInit) return;

	s_frameCount++;

	if (s_frameCount % 60 != 0) return;

	Pool_Compact(&s_elementPool);
	Pool_Compact(&s_vnodePool);
	Pool_Compact(&s_componentPool);
	Pool_Compact(&s_childPool);
	Pool_Compact(&s_drawableRectPool);
	Pool_Compact(&s_drawableImagePool);
	Pool_Compact(&s_drawableTextPool);
	Pool_Compact(&s_drawableSplitterPool);
	Pool_Compact(&s_gpuObjectPool);
}

SableMemory::SizeData SableMemory::GetSizeData(PoolType type)
{
	auto fillData = [](SizeData& data, const DynamicPool* pool) {
		data.numChunks = pool->chunks.size();
		data.peak = pool->peakUsage;
		data.totalUsed = 0;
		data.totalCapacity = 0;

		for (const auto& chunk : pool->chunks)
		{
			data.totalCapacity += chunk.capacity;
			data.totalUsed += chunk.used;
		}

		data.sizeInKB = (data.totalCapacity * pool->objectSize) / 1024;
	};

	SizeData data{};

	switch (type)
	{
	case PoolType::Element:
		fillData(data, &s_elementPool);
		break;
	case PoolType::VirtualNode:
		fillData(data, &s_vnodePool);
		break;
	case PoolType::BaseComponent:
		fillData(data, &s_componentPool);
		break;
	case PoolType::Child:
		fillData(data, &s_childPool);
		break;
	case PoolType::DrawableRect:
		fillData(data, &s_drawableRectPool);
		break;
	case PoolType::DrawableImage:
		fillData(data, &s_drawableImagePool);
		break;
	case PoolType::DrawableText:
		fillData(data, &s_drawableTextPool);
		break;
	case PoolType::DrawableSplitter:
		fillData(data, &s_drawableSplitterPool);
		break;
	case PoolType::GpuObject:
		fillData(data, &s_gpuObjectPool);
		break;
	default:
		break;
	}

	return data;
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
	Pool_Destroy(&s_drawableSplitterPool);
	Pool_Destroy(&s_gpuObjectPool);

	s_poolsInit = false;
	s_frameCount = 0;
}