#pragma once
#include <SableUI/types/renderer_types.h>
#include <cstdint>

namespace SableUI
{
	struct GpuObjectMetadata
	{
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
	};

	class RendererBackend;
	struct GpuObject
	{
		static int GetNumInstances();
		GpuObject();
		~GpuObject();
		RendererBackend* context = nullptr;
		uint32_t handle = 0;
		uint32_t numVertices = 0;
		uint32_t numIndices = 0;
		VertexLayout layout{};
	};
}