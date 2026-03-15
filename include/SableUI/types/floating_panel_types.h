#pragma once
#include <SableUI/utils/utils.h>
#include <SableUI/renderer/resource_handle.h>
#include <SableUI/renderer/command_buffer.h>
#include <SableUI/states/state_base.h>
#include <SableUI/core/drawable.h>
#include <SableUI/core/events.h>

namespace SableUI
{
	class FloatingPanelBase : public StateBase
	{
	public:
		virtual ~FloatingPanelBase() = default;
		virtual bool IsOpen() const = 0;

		virtual void PostLayoutUpdate(const UIEventContext& ctx) = 0;
		virtual bool CheckAndUpdate(const DrawableDrawData& externalDrawData) = 0;
		virtual void HandleInput(const UIEventContext& ctx) = 0;
	};

	struct FloatingPanelEntry {
		FloatingPanelBase* panel;
		ivec2 pos;
		ivec2 size;
		int zIndex;
		ResourceHandle texture;
		ResourceHandle framebuffer;
		CommandBuffer cmd;
		bool dirty = true;
	};
}