#pragma once
#include "Core/Renderer/Model.h"
#include "Core/Renderer/Material.h"
#include "Core/Editor/Viewport.h"
#include <imgui.h>
#include <memory>
#include <vector>
#include <string>

namespace Editor
{
    // Visual model viewer panel with real-time preview
    class ModelPanel
    {
    public:
        ModelPanel();
        ~ModelPanel() = default;

        void OnImGuiRender();
        
        // Load a model to view
        void LoadModel(const std::string& path);
        std::shared_ptr<Core::Renderer::Model> GetModel() const { return m_CurrentModel; }
        
        // Enable/disable panel
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }
        
        // Live preview control
        void SetLivePreview(bool enabled) { m_LivePreview = enabled; }
        bool IsLivePreview() const { return m_LivePreview; }

    private:
        void RenderModelInfo();
        void RenderMeshList();
        void RenderMaterialInfo();
        void RenderLivePreview();
        void RenderControls();
        void RenderStatistics();
        
        // Helper to load model from file dialog
        void OpenFileDialog();

    private:
        bool m_Enabled = true;
        bool m_LivePreview = true;
        
        std::shared_ptr<Core::Renderer::Model> m_CurrentModel;
        std::string m_CurrentModelPath;
        
        // Preview viewport
        std::unique_ptr<Core::Editor::Viewport> m_Viewport;
        std::shared_ptr<Core::Renderer::Material> m_PreviewMaterial;
        
        // Preview controls
        enum class PreviewShape { Sphere, Cube, LoadedModel };
        PreviewShape m_PreviewShape = PreviewShape::LoadedModel;
        float m_ModelRotation = 0.0f;
        bool m_AutoRotate = true;
        float m_ModelScale = 1.0f;
        glm::vec3 m_ModelOffset = glm::vec3(0.0f);
        
        // Mesh selection
        int m_SelectedMeshIndex = -1;
        
        // Display options
        bool m_ShowWireframe = false;
        bool m_ShowNormals = false;
        bool m_ShowBoundingBox = false;
        
        // Camera controls
        float m_CameraDistance = 5.0f;
        float m_CameraYaw = 0.0f;
        float m_CameraPitch = 0.0f;
        
        // Recent models
        std::vector<std::string> m_RecentModels;
        static constexpr int MaxRecentModels = 10;
    };
}
