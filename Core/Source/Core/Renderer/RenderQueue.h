#pragma once
#include <mutex>
#include <vector>

#include "Core/Renderer/RenderCommand.h"

namespace Core::Renderer
{
    class RenderQueue
    {
    public:
        RenderQueue() = default;

        // Called from ANY thread
        void Submit(RenderCommandBuffer&& buffer);

        // Called on render thread (once per frame typically)
        void Execute();

        // Context setup (render thread or init thread)
        RenderCommandContext& GetContext() { return m_Context; }
        const RenderCommandContext& GetContext() const { return m_Context; }

    private:
        RenderCommandContext m_Context;

        std::mutex m_Mutex;
        std::vector<RenderCommandBuffer> m_Pending; // incoming from workers
    };
}
