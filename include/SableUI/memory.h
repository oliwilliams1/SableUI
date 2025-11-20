#pragma once
#include <cstddef>
#include <utility>

namespace SableMemory
{
	void* SB_alloc(size_t size);
	void SB_free(void* ptr);

	template <typename T, typename... Args>
	T* SB_new(Args&&... args)
	{
		void* p = SB_alloc(sizeof(T));
		if (!p) return nullptr;
		return new (p) T(std::forward<Args>(args)...);
	}

	template <typename T>
	void SB_delete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			SB_free(ptr);
		}
	}

	void DestroyPools();
	void InitPools();
	void CompactPools();

	enum class PoolType
	{
		Element,
		VirtualNode,
		BaseComponent,
		Child,
		DrawableRect,
		DrawableImage,
		DrawableText
	};

	struct SizeData {
		size_t numChunks;
		size_t totalUsed;
		size_t totalCapacity;
		size_t peak;
		size_t sizeInKB;
	};

	SizeData GetSizeData(PoolType type);
}