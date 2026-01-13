#pragma once
#include "Core/Layer.h"
#include <imgui.h>
#include <vector>
#include <string>
#include <cstdint>

namespace Editor
{
    // Performance metrics tracking
    struct PerformanceMetrics
    {
        float FrameTime = 0.0f;
        float FPS = 0.0f;
        
        // Memory metrics
        uint64_t AllocatedMemory = 0;
        uint64_t UsedMemory = 0;
        uint64_t FreeMemory = 0;
        
        // Rendering metrics
        uint32_t DrawCalls = 0;
        uint32_t Triangles = 0;
        uint32_t Vertices = 0;
        
        // Custom metrics
        std::vector<std::pair<std::string, float>> CustomMetrics;
    };

    // Profiler panel with Tracy integration
    class ProfilerPanel
    {
    public:
        ProfilerPanel();
        ~ProfilerPanel() = default;

        void OnImGuiRender();
        
        // Update metrics (call once per frame)
        void UpdateMetrics(const PerformanceMetrics& metrics);
        
        // Add custom metric
        void AddCustomMetric(const std::string& name, float value);
        
        // Enable/disable panel
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

    private:
        void RenderFrameTimeGraph();
        void RenderMemoryInfo();
        void RenderRenderingStats();
        void RenderCustomMetrics();
        void RenderTracyInfo();

    private:
        bool m_Enabled = true;
        PerformanceMetrics m_CurrentMetrics;
        
        // Frame time history for graph
        static constexpr size_t HistorySize = 120;
        std::vector<float> m_FrameTimeHistory;
        size_t m_HistoryIndex = 0;
        
        // Display options
        bool m_ShowFrameTimeGraph = true;
        bool m_ShowMemoryInfo = true;
        bool m_ShowRenderingStats = true;
        bool m_ShowCustomMetrics = true;
        bool m_ShowTracyInfo = true;
    };
}
