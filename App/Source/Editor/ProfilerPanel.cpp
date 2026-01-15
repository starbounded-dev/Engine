#include "ProfilerPanel.h"
#include "Core/Debug/Profiler.h"
#include <algorithm>

namespace Editor
{
    ProfilerPanel::ProfilerPanel()
    {
        m_FrameTimeHistory.resize(HistorySize, 0.0f);
    }

    void ProfilerPanel::OnImGuiRender()
    {
        PROFILE_FUNC();

        if (!m_Enabled)
            return;

        ImGui::Begin("Profiler", &m_Enabled);

        // Display options
        ImGui::Text("Display Options:");
        ImGui::Checkbox("Frame Time Graph", &m_ShowFrameTimeGraph);
        ImGui::SameLine();
        ImGui::Checkbox("Memory Info", &m_ShowMemoryInfo);
        ImGui::Checkbox("Rendering Stats", &m_ShowRenderingStats);
        ImGui::SameLine();
        ImGui::Checkbox("Custom Metrics", &m_ShowCustomMetrics);
        ImGui::Checkbox("Tracy Info", &m_ShowTracyInfo);
        
        ImGui::Separator();

        // Current frame stats
        ImGui::Text("Frame Time: %.2f ms", m_CurrentMetrics.FrameTime);
        ImGui::SameLine();
        ImGui::Text("FPS: %.1f", m_CurrentMetrics.FPS);

        if (m_ShowFrameTimeGraph)
        {
            ImGui::Separator();
            RenderFrameTimeGraph();
        }

        if (m_ShowMemoryInfo)
        {
            ImGui::Separator();
            RenderMemoryInfo();
        }

        if (m_ShowRenderingStats)
        {
            ImGui::Separator();
            RenderRenderingStats();
        }

        if (m_ShowCustomMetrics && !m_CurrentMetrics.CustomMetrics.empty())
        {
            ImGui::Separator();
            RenderCustomMetrics();
        }

        if (m_ShowTracyInfo)
        {
            ImGui::Separator();
            RenderTracyInfo();
        }

        ImGui::End();
    }

    void ProfilerPanel::UpdateMetrics(const PerformanceMetrics& metrics)
    {
        m_CurrentMetrics = metrics;
        
        // Update frame time history
        m_FrameTimeHistory[m_HistoryIndex] = metrics.FrameTime;
        m_HistoryIndex = (m_HistoryIndex + 1) % HistorySize;
    }

    void ProfilerPanel::AddCustomMetric(const std::string& name, float value)
    {
        // Check if metric already exists
        for (auto& metric : m_CurrentMetrics.CustomMetrics)
        {
            if (metric.first == name)
            {
                metric.second = value;
                return;
            }
        }
        
        // Add new metric
        m_CurrentMetrics.CustomMetrics.push_back({name, value});
    }

    void ProfilerPanel::RenderFrameTimeGraph()
    {
        ImGui::Text("Frame Time History:");
        
        // Calculate min/max for better scaling
        float minTime = *std::min_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        float maxTime = *std::max_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        
        // Add some padding
        maxTime = std::max(maxTime * 1.1f, 16.67f); // At least show up to 60 FPS target
        
        ImGui::PlotLines("##FrameTime", 
            m_FrameTimeHistory.data(), 
            static_cast<int>(m_FrameTimeHistory.size()),
            static_cast<int>(m_HistoryIndex),
            nullptr,
            minTime,
            maxTime,
            ImVec2(0, 80));
        
        // Target frame time indicators
        ImGui::Text("Target: 16.67ms (60 FPS) | 33.33ms (30 FPS)");
    }

    void ProfilerPanel::RenderMemoryInfo()
    {
        ImGui::Text("Memory Information:");
        
        auto formatBytes = [](uint64_t bytes) -> std::string {
            if (bytes < 1024)
                return std::to_string(bytes) + " B";
            else if (bytes < 1024 * 1024)
                return std::to_string(bytes / 1024) + " KB";
            else if (bytes < 1024 * 1024 * 1024)
                return std::to_string(bytes / (1024 * 1024)) + " MB";
            else
                return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
        };

        ImGui::Text("Allocated: %s", formatBytes(m_CurrentMetrics.AllocatedMemory).c_str());
        ImGui::Text("Used:      %s", formatBytes(m_CurrentMetrics.UsedMemory).c_str());
        ImGui::Text("Free:      %s", formatBytes(m_CurrentMetrics.FreeMemory).c_str());
        
        // Memory usage bar
        if (m_CurrentMetrics.AllocatedMemory > 0)
        {
            float usage = static_cast<float>(m_CurrentMetrics.UsedMemory) / 
                         static_cast<float>(m_CurrentMetrics.AllocatedMemory);
            ImGui::ProgressBar(usage, ImVec2(-1, 0), 
                (std::to_string(static_cast<int>(usage * 100)) + "%").c_str());
        }
    }

    void ProfilerPanel::RenderRenderingStats()
    {
        ImGui::Text("Rendering Statistics:");
        ImGui::Text("Draw Calls: %u", m_CurrentMetrics.DrawCalls);
        ImGui::Text("Triangles:  %u", m_CurrentMetrics.Triangles);
        ImGui::Text("Vertices:   %u", m_CurrentMetrics.Vertices);
    }

    void ProfilerPanel::RenderCustomMetrics()
    {
        ImGui::Text("Custom Metrics:");
        
        for (const auto& [name, value] : m_CurrentMetrics.CustomMetrics)
        {
            ImGui::Text("%s: %.2f", name.c_str(), value);
        }
    }

    void ProfilerPanel::RenderTracyInfo()
    {
        ImGui::Text("Tracy Profiler:");
        
#if ENABLE_PROFILING
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Profiling ENABLED");
        ImGui::Text("Connect Tracy Profiler to see detailed profiling data");
        ImGui::Text("Zones are being captured for all PROFILE_FUNC() calls");
#else
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Profiling DISABLED");
        ImGui::Text("Build in Debug or Release mode to enable profiling");
#endif
        
        ImGui::Separator();
        ImGui::Text("Tips:");
        ImGui::BulletText("Use PROFILE_FUNC() at the start of functions");
        ImGui::BulletText("Use PROFILE_SCOPE() for specific code blocks");
        ImGui::BulletText("Launch Tracy Profiler application to connect");
    }
}
