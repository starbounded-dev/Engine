#pragma once
#include "Core/Layer.h"
#include "Core/Renderer/ShaderManager.h"
#include "Core/Renderer/ShaderEditorInterface.h"
#include <imgui.h>
#include <string>
#include <vector>
#include <filesystem>

namespace Editor
{
    // Shader editor with syntax highlighting and live preview
    class ShaderEditor : public Core::Renderer::IShaderEditorInterface
    {
    public:
        ShaderEditor();
        ~ShaderEditor() = default;

        void OnImGuiRender();
        
        // Load shader for editing (IShaderEditorInterface implementation)
        void LoadShader(const std::string& name);
        void LoadShaderFiles(const std::filesystem::path& vertexPath, 
                            const std::filesystem::path& fragmentPath) override;
        
        // Enable/disable panel
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }
        
        // Global instance access (set by ImLayer)
        static void SetInstance(ShaderEditor* instance);
        static ShaderEditor* GetInstance();

    private:
        void RenderMenuBar();
        void RenderShaderList();
        void RenderEditor();
        void RenderErrorDisplay();
        void RenderPreview();
        void RenderStatusBar();
        
        // Shader operations
        void SaveCurrentShader();
        void ReloadCurrentShader();
        void CompileAndTest();
        
        // Syntax highlighting (basic)
        void ApplyBasicSyntaxHighlighting(std::string& text);
        
        // File operations
        bool LoadFileContent(const std::filesystem::path& path, std::string& content);
        bool SaveFileContent(const std::filesystem::path& path, const std::string& content);

    private:
        bool m_Enabled = true;
        
        // Current shader being edited
        std::string m_CurrentShaderName;
        std::filesystem::path m_CurrentVertexPath;
        std::filesystem::path m_CurrentFragmentPath;
        
        // Editor buffers
        char m_VertexShaderBuffer[1024 * 16]; // 16KB for vertex shader
        char m_FragmentShaderBuffer[1024 * 16]; // 16KB for fragment shader
        
        // Editor state
        bool m_IsVertexShaderModified = false;
        bool m_IsFragmentShaderModified = false;
        bool m_ShowVertexShader = true;
        bool m_ShowFragmentShader = true;
        
        // Compilation state
        bool m_HasCompilationError = false;
        std::string m_CompilationError;
        std::string m_LastSuccessfulCompile;
        
        // Preview state
        bool m_EnableLivePreview = false;
        bool m_AutoReloadOnSave = true;
        
        // Available shaders in the manager
        std::vector<std::string> m_AvailableShaders;
        int m_SelectedShaderIndex = -1;
        
        // UI state
        bool m_ShowShaderList = true;
        bool m_ShowErrorDisplay = true;
        bool m_ShowPreview = false;
        float m_ShaderListWidth = 200.0f;
        
        // Static instance for global access
        static ShaderEditor* s_Instance;
    };
}
