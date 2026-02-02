#pragma once
#include <SableUI/states/state_base.h>
#include <SableUI/core/events.h>
namespace SableUI
{
    class FloatingPanelStateBase : public StateBase
    {
    public:
        virtual ~FloatingPanelStateBase() = default;
        virtual void HandleInput(const UIEventContext& ctx) = 0;
        virtual void Layout() = 0;
        virtual void PostLayoutUpdate(const UIEventContext& ctx) = 0;
        virtual bool IsOpen() const = 0;
    };
}