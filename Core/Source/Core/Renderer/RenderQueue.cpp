#include "RenderQueue.h"

namespace Core::Renderer
{
    void RenderQueue::Submit(RenderCommandBuffer&& buffer)
    {
        if (buffer.Empty())
            return;

        std::scoped_lock lock(m_Mutex);
        m_Pending.emplace_back(std::move(buffer));
    }

    void RenderQueue::Execute()
    {
        // Swap pending into local list to minimize lock time
        std::vector<RenderCommandBuffer> local;
        {
            std::scoped_lock lock(m_Mutex);
            local.swap(m_Pending);
        }

        // Execute in submission order
        for (auto& buf : local)
            buf.Execute(m_Context);

        // Optional: reset state each frame
        m_Context.ResetState();
    }
}
