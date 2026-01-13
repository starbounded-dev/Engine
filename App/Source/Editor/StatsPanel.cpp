#include "StatsPanel.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Editor
{
    StatsPanel::StatsPanel()
    {
        m_FrameTimeHistory.resize(HistorySize, 0.0f);
    }

    void StatsPanel::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        ImGui::Begin("Renderer Statistics", &m_Enabled);

        // Display options
        if (ImGui::CollapsingHeader("Display Options", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Draw Call Stats", &m_ShowDrawCallStats);
            ImGui::Checkbox("Memory Stats", &m_ShowMemoryStats);
            ImGui::Checkbox("Frame Time Graph", &m_ShowFrameTimeGraph);
            ImGui::Checkbox("Render Pass Info", &m_ShowRenderPassInfo);
            
            if (ImGui::Button("Reset Peak Values"))
            {
                m_PeakStats = m_CurrentStats;
            }
        }

        ImGui::Separator();

        // Render sections
        if (m_ShowDrawCallStats)
            RenderDrawCallStats();
        
        if (m_ShowMemoryStats)
            RenderMemoryStats();
        
        if (m_ShowFrameTimeGraph)
            RenderFrameTimeGraph();
        
        if (m_ShowRenderPassInfo)
            RenderRenderPassInfo();

        ImGui::End();
    }

    void StatsPanel::UpdateStats(const RendererStats& stats)
    {
        m_CurrentStats = stats;
        m_FrameCount++;
        
        // Update peak stats
        m_PeakStats.DrawCalls = std::max(m_PeakStats.DrawCalls, stats.DrawCalls);
        m_PeakStats.TriangleCount = std::max(m_PeakStats.TriangleCount, stats.TriangleCount);
        m_PeakStats.VertexCount = std::max(m_PeakStats.VertexCount, stats.VertexCount);
        m_PeakStats.TextureMemoryUsed = std::max(m_PeakStats.TextureMemoryUsed, stats.TextureMemoryUsed);
        m_PeakStats.BufferMemoryUsed = std::max(m_PeakStats.BufferMemoryUsed, stats.BufferMemoryUsed);
        
        // Update frame time history
        m_FrameTimeHistory[m_HistoryIndex] = stats.FrameTime;
        m_HistoryIndex = (m_HistoryIndex + 1) % HistorySize;
    }

    void StatsPanel::ResetStats()
    {
        m_CurrentStats = RendererStats();
        m_PeakStats = RendererStats();
        m_FrameCount = 0;
        std::fill(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end(), 0.0f);
        m_HistoryIndex = 0;
    }

    void StatsPanel::RenderDrawCallStats()
    {
        if (ImGui::CollapsingHeader("Draw Call Statistics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Draw Calls: %u (Peak: %u)", m_CurrentStats.DrawCalls, m_PeakStats.DrawCalls);
            ImGui::Text("Triangles: %s (Peak: %s)", 
                std::to_string(m_CurrentStats.TriangleCount).c_str(),
                std::to_string(m_PeakStats.TriangleCount).c_str());
            ImGui::Text("Vertices: %s (Peak: %s)", 
                std::to_string(m_CurrentStats.VertexCount).c_str(),
                std::to_string(m_PeakStats.VertexCount).c_str());
            ImGui::Text("Indices: %u", m_CurrentStats.IndexCount);
            
            ImGui::Separator();
            
            // Derived metrics
            if (m_CurrentStats.DrawCalls > 0)
            {
                uint32_t avgTrisPerCall = m_CurrentStats.TriangleCount / m_CurrentStats.DrawCalls;
                ImGui::Text("Avg Triangles/Draw: %u", avgTrisPerCall);
            }
            
            if (m_CurrentStats.TriangleCount > 0)
            {
                float trisPerSecond = m_CurrentStats.TriangleCount * m_CurrentStats.FPS;
                ImGui::Text("Triangles/Second: %.2fM", trisPerSecond / 1000000.0f);
            }
        }
    }

    void StatsPanel::RenderMemoryStats()
    {
        if (ImGui::CollapsingHeader("Memory Statistics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Texture memory
            ImGui::Text("Texture Memory");
            ImGui::Indent();
            ImGui::Text("Used: %s", FormatBytes(m_CurrentStats.TextureMemoryUsed).c_str());
            ImGui::Text("Allocated: %s", FormatBytes(m_CurrentStats.TextureMemoryAllocated).c_str());
            ImGui::Text("Textures: %u", m_CurrentStats.TextureCount);
            
            // Progress bar for texture memory
            if (m_CurrentStats.TextureMemoryAllocated > 0)
            {
                float usage = (float)m_CurrentStats.TextureMemoryUsed / (float)m_CurrentStats.TextureMemoryAllocated;
                ImGui::ProgressBar(usage, ImVec2(-1.0f, 0.0f), 
                    (std::to_string((int)(usage * 100)) + "%").c_str());
            }
            ImGui::Unindent();
            
            ImGui::Separator();
            
            // Buffer memory
            ImGui::Text("Buffer Memory");
            ImGui::Indent();
            ImGui::Text("Used: %s", FormatBytes(m_CurrentStats.BufferMemoryUsed).c_str());
            ImGui::Text("Vertex Buffers: %u", m_CurrentStats.VertexBufferCount);
            ImGui::Text("Index Buffers: %u", m_CurrentStats.IndexBufferCount);
            ImGui::Text("Uniform Buffers: %u", m_CurrentStats.UniformBufferCount);
            ImGui::Unindent();
            
            ImGui::Separator();
            
            // Total memory
            uint64_t totalMemory = m_CurrentStats.TextureMemoryUsed + m_CurrentStats.BufferMemoryUsed;
            ImGui::Text("Total GPU Memory: %s", FormatBytes(totalMemory).c_str());
        }
    }

    void StatsPanel::RenderFrameTimeGraph()
    {
        if (ImGui::CollapsingHeader("Frame Time", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("FPS: %.1f", m_CurrentStats.FPS);
            ImGui::Text("Frame Time: %.2f ms", m_CurrentStats.FrameTime);
            
            // Calculate min/max/avg from history
            float minTime = *std::min_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
            float maxTime = *std::max_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
            float avgTime = 0.0f;
            for (float time : m_FrameTimeHistory)
                avgTime += time;
            avgTime /= HistorySize;
            
            ImGui::Text("Min: %.2f ms | Avg: %.2f ms | Max: %.2f ms", minTime, avgTime, maxTime);
            
            // Frame time graph
            ImGui::PlotLines("Frame Time (ms)", 
                m_FrameTimeHistory.data(), 
                (int)HistorySize, 
                (int)m_HistoryIndex,
                nullptr,
                0.0f,
                maxTime * 1.2f,
                ImVec2(0, 80));
            
            // Target frame time indicators
            ImGui::Separator();
            ImGui::Text("Target Frame Times:");
            ImGui::SameLine();
            ImGui::TextColored(m_CurrentStats.FrameTime <= 16.67f ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "60fps");
            ImGui::SameLine();
            ImGui::TextColored(m_CurrentStats.FrameTime <= 33.33f ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "30fps");
        }
    }

    void StatsPanel::RenderRenderPassInfo()
    {
        if (ImGui::CollapsingHeader("Render Pass Info", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Render Passes: %u", m_CurrentStats.RenderPasses);
            ImGui::Text("Shader Switches: %u", m_CurrentStats.ShaderSwitches);
            
            // Efficiency metrics
            if (m_CurrentStats.RenderPasses > 0)
            {
                float drawCallsPerPass = (float)m_CurrentStats.DrawCalls / (float)m_CurrentStats.RenderPasses;
                ImGui::Text("Avg Draw Calls/Pass: %.1f", drawCallsPerPass);
            }
            
            if (m_CurrentStats.ShaderSwitches > 0)
            {
                float drawCallsPerSwitch = (float)m_CurrentStats.DrawCalls / (float)m_CurrentStats.ShaderSwitches;
                ImGui::Text("Avg Draw Calls/Shader: %.1f", drawCallsPerSwitch);
            }
        }
    }

    std::string StatsPanel::FormatBytes(uint64_t bytes) const
    {
        const char* units[] = { "B", "KB", "MB", "GB", "TB" };
        int unitIndex = 0;
        double value = static_cast<double>(bytes);
        
        while (value >= 1024.0 && unitIndex < 4)
        {
            value /= 1024.0;
            unitIndex++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value << " " << units[unitIndex];
        return oss.str();
    }
}
