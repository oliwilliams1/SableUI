#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/states/state_base.h>

namespace SableUI
{
	class FloatingPanelBase : public StateBase
	{
	public:
		virtual ~FloatingPanelBase() = default;
	};

	struct FloatingPanelEntry {
		FloatingPanelBase* panel;
		ivec2 pos;
		ivec2 size;
		int zIndex;
		ResourceHandle texture;
		ResourceHandle framebuffer;
		CommandBuffer cb;
		bool dirty = true;
	};
}