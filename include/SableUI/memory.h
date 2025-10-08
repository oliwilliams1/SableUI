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
}