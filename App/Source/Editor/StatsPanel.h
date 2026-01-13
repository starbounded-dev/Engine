#pragma once
#include "Core/Layer.h"
#include <imgui.h>
#include <vector>
#include <string>
#include <cstdint>

namespace Editor
{
    // Renderer statistics for display
    struct RendererStats
    {
        // Draw call statistics
        uint32_t DrawCalls = 0;
        uint32_t TriangleCount = 0;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
        
        // Texture memory tracking
        uint64_t TextureMemoryUsed = 0;      // Bytes
        uint64_t TextureMemoryAllocated = 0; // Bytes
        uint32_t TextureCount = 0;
        
        // Buffer memory tracking
        uint64_t BufferMemoryUsed = 0;
        uint32_t VertexBufferCount = 0;
        uint32_t IndexBufferCount = 0;
        uint32_t UniformBufferCount = 0;
        
        // Frame time
        float FrameTime = 0.0f;  // ms
        float FPS = 0.0f;
        
        // Render pass info
        uint32_t RenderPasses = 0;
        uint32_t ShaderSwitches = 0;
    };

    // Renderer statistics panel
    class StatsPanel
    {
    public:
        StatsPanel();
        ~StatsPanel() = default;

        void OnImGuiRender();
        
        // Update statistics (call once per frame)
        void UpdateStats(const RendererStats& stats);
        
        // Reset statistics
        void ResetStats();
        
        // Enable/disable panel
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

    private:
        void RenderDrawCallStats();
        void RenderMemoryStats();
        void RenderFrameTimeGraph();
        void RenderRenderPassInfo();
        
        std::string FormatBytes(uint64_t bytes) const;

    private:
        bool m_Enabled = true;
        RendererStats m_CurrentStats;
        RendererStats m_PeakStats;  // Track peak values
        
        // Frame time history for graph
        static constexpr size_t HistorySize = 120;
        std::vector<float> m_FrameTimeHistory;
        size_t m_HistoryIndex = 0;
        
        // Display options
        bool m_ShowDrawCallStats = true;
        bool m_ShowMemoryStats = true;
        bool m_ShowFrameTimeGraph = true;
        bool m_ShowRenderPassInfo = true;
        
        // Track frame count for averaging
        uint32_t m_FrameCount = 0;
    };
}
