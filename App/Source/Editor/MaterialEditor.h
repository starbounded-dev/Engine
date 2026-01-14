#pragma once
#include "Core/Layer.h"
#include "Core/Renderer/Material.h"
#include "Core/Editor/Viewport.h"
#include <imgui.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace Editor
{
    // Material template for quick material creation
    struct MaterialTemplate
    {
        std::string Name;
        std::string VertexShader;
        std::string FragmentShader;
        std::unordered_map<std::string, Core::Renderer::MaterialValue> DefaultValues;
        std::vector<std::string> TextureSlots; // Names of texture slots (e.g., "u_Albedo", "u_Normal")
    };

    // Visual material editor panel
    class MaterialEditor
    {
    public:
        MaterialEditor();
        ~MaterialEditor() = default;

        void OnImGuiRender();
        
        // Set the material to edit
        void SetMaterial(std::shared_ptr<Core::Renderer::Material> material);
        std::shared_ptr<Core::Renderer::Material> GetMaterial() const { return m_CurrentMaterial; }
        
        // Material templates
        void AddTemplate(const MaterialTemplate& tmpl);
        const std::vector<MaterialTemplate>& GetTemplates() const { return m_Templates; }
        
        // Enable/disable panel
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }
        
        // Live preview control
        void SetLivePreview(bool enabled) { m_LivePreview = enabled; }
        bool IsLivePreview() const { return m_LivePreview; }

    private:
        void RenderMaterialSelector();
        void RenderPropertyEditor();
        void RenderTextureSlots();
        void RenderTemplateSelector();
        void RenderLivePreview();
        void RenderActions();
        
        void CreateMaterialFromTemplate(const MaterialTemplate& tmpl);
        void SaveMaterial();
        void LoadMaterial();
        
        // Helper to render material value editor
        void RenderMaterialValueEditor(const std::string& name, Core::Renderer::MaterialValue& value);

    private:
        bool m_Enabled = true;
        bool m_LivePreview = true;
        
        std::shared_ptr<Core::Renderer::Material> m_CurrentMaterial;
        std::vector<MaterialTemplate> m_Templates;
        
        // Material library (for selector)
        std::vector<std::shared_ptr<Core::Renderer::Material>> m_MaterialLibrary;
        int m_SelectedMaterialIndex = -1;
        
        // Texture slot management
        struct TextureSlotUI
        {
            std::string SlotName;
            uint32_t TextureUnit = 0;
            GLuint CurrentTextureID = 0;
            char FilePath[256] = "";
        };
        std::vector<TextureSlotUI> m_TextureSlots;
        
        // Template creation UI
        bool m_ShowTemplateCreator = false;
        char m_NewTemplateName[128] = "";
        char m_NewTemplateVertPath[256] = "";
        char m_NewTemplateFragPath[256] = "";
        
        // Material creation UI
        bool m_ShowMaterialCreator = false;
        char m_NewMaterialName[128] = "";
        int m_SelectedTemplateIndex = -1;
        
        // Preview state
        bool m_ShowPreviewWindow = false;
        float m_PreviewRotation = 0.0f;
        std::unique_ptr<Core::Editor::Viewport> m_PreviewViewport;
        enum class PreviewShape { Sphere, Cube } m_PreviewShape = PreviewShape::Sphere;
    };
}
