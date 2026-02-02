#pragma once

namespace SableUI
{
    class StateBase
    {
    public:
        virtual ~StateBase() = default;
        virtual void Sync(StateBase* other) = 0;
    };
}