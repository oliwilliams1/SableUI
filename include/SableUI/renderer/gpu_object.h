#pragma once
#include <SableUI/types/renderer_types.h>
#include <cstdint>

namespace SableUI
{
	class RendererBackend;
	struct GpuObject
	{
		static int GetNumInstances();
		GpuObject();
		~GpuObject();
		RendererBackend* context = nullptr;
		uint32_t handle = 0;
		uint32_t vbo = 0;
		uint32_t ebo = 0;
		uint32_t numVertices = 0;
		uint32_t numIndices = 0;
		VertexLayout layout{};
	};
}