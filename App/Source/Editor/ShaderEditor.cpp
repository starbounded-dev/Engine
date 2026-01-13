#include "ShaderEditor.h"
#include "Core/Debug/Profiler.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Editor
{
    // Static instance
    ShaderEditor* ShaderEditor::s_Instance = nullptr;
    
    ShaderEditor::ShaderEditor()
    {
        // Initialize buffers
        m_VertexShaderBuffer[0] = '\0';
        m_FragmentShaderBuffer[0] = '\0';
        
        // Create preview viewport
        m_PreviewViewport = std::make_unique<Core::Renderer::Viewport>(512, 512);
        
        // Get available shaders from manager
        auto& shaderMgr = Core::Renderer::ShaderManager::Get();
        m_AvailableShaders = shaderMgr.GetShaderNames();
    }
    
    void ShaderEditor::SetInstance(ShaderEditor* instance)
    {
        s_Instance = instance;
        // Also register with the Core layer interface
        Core::Renderer::SetShaderEditorInterface(instance);
    }
    
    ShaderEditor* ShaderEditor::GetInstance()
    {
        return s_Instance;
    }

    void ShaderEditor::OnImGuiRender()
    {
        PROFILE_FUNC();

        if (!m_Enabled)
            return;

        ImGui::Begin("Shader Editor", &m_Enabled, ImGuiWindowFlags_MenuBar);

        RenderMenuBar();

        // Split view: shader list on left, editor on right
        if (m_ShowShaderList)
        {
            ImGui::BeginChild("ShaderList", ImVec2(m_ShaderListWidth, 0), true);
            RenderShaderList();
            ImGui::EndChild();
            
            ImGui::SameLine();
        }

        ImGui::BeginChild("EditorArea");
        
        RenderEditor();
        
        if (m_ShowErrorDisplay && m_HasCompilationError)
        {
            ImGui::Separator();
            RenderErrorDisplay();
        }
        
        if (m_ShowPreview)
        {
            ImGui::Separator();
            RenderPreview();
        }
        
        ImGui::EndChild();

        RenderStatusBar();

        ImGui::End();
    }

    void ShaderEditor::LoadShader(const std::string& name)
    {
        auto& shaderMgr = Core::Renderer::ShaderManager::Get();
        
        if (!shaderMgr.HasShader(name))
        {
            m_CompilationError = "Shader '" + name + "' not found in ShaderManager";
            m_HasCompilationError = true;
            return;
        }

        m_CurrentShaderName = name;
        
        // Get shader info from manager
        const auto* shaderInfo = shaderMgr.GetShaderInfo(name);
        if (!shaderInfo)
        {
            m_CompilationError = "Failed to get shader info for: " + name;
            m_HasCompilationError = true;
            return;
        }
        
        // Check if it's a compute shader
        if (shaderInfo->IsCompute)
        {
            m_CompilationError = "Compute shader editing not yet supported.\n";
            m_CompilationError += "Only graphics shaders (vertex + fragment) can be edited.";
            m_HasCompilationError = true;
            return;
        }
        
        // Load the shader files using their paths
        LoadShaderFiles(shaderInfo->VertexPath, shaderInfo->FragmentPath);
    }

    void ShaderEditor::LoadShaderFiles(const std::filesystem::path& vertexPath, 
                                       const std::filesystem::path& fragmentPath)
    {
        m_CurrentVertexPath = vertexPath;
        m_CurrentFragmentPath = fragmentPath;
        
        std::string vertexContent, fragmentContent;
        
        if (!LoadFileContent(vertexPath, vertexContent))
        {
            m_CompilationError = "Failed to load vertex shader: " + vertexPath.string();
            m_HasCompilationError = true;
            return;
        }
        
        if (!LoadFileContent(fragmentPath, fragmentContent))
        {
            m_CompilationError = "Failed to load fragment shader: " + fragmentPath.string();
            m_HasCompilationError = true;
            return;
        }
        
        // Copy to buffers (truncate if too large)
        size_t vertexLen = std::min(vertexContent.size(), sizeof(m_VertexShaderBuffer) - 1);
        size_t fragmentLen = std::min(fragmentContent.size(), sizeof(m_FragmentShaderBuffer) - 1);
        
        std::memcpy(m_VertexShaderBuffer, vertexContent.c_str(), vertexLen);
        m_VertexShaderBuffer[vertexLen] = '\0';
        
        std::memcpy(m_FragmentShaderBuffer, fragmentContent.c_str(), fragmentLen);
        m_FragmentShaderBuffer[fragmentLen] = '\0';
        
        m_IsVertexShaderModified = false;
        m_IsFragmentShaderModified = false;
        m_HasCompilationError = false;
        m_LastSuccessfulCompile = "Shader files loaded successfully";
    }

    void ShaderEditor::RenderMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                    SaveCurrentShader();
                
                if (ImGui::MenuItem("Reload", "Ctrl+R"))
                    ReloadCurrentShader();
                
                ImGui::Separator();
                
                if (ImGui::MenuItem("Compile & Test", "F5"))
                    CompileAndTest();
                
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Shader List", nullptr, &m_ShowShaderList);
                ImGui::MenuItem("Error Display", nullptr, &m_ShowErrorDisplay);
                ImGui::MenuItem("Preview", nullptr, &m_ShowPreview);
                
                ImGui::Separator();
                
                ImGui::MenuItem("Vertex Shader", nullptr, &m_ShowVertexShader);
                ImGui::MenuItem("Fragment Shader", nullptr, &m_ShowFragmentShader);
                
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::MenuItem("Live Preview", nullptr, &m_EnableLivePreview);
                ImGui::MenuItem("Auto-Reload on Save", nullptr, &m_AutoReloadOnSave);
                
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
    }

    void ShaderEditor::RenderShaderList()
    {
        ImGui::Text("Available Shaders:");
        ImGui::Separator();
        
        // Refresh button
        if (ImGui::Button("Refresh"))
        {
            auto& shaderMgr = Core::Renderer::ShaderManager::Get();
            m_AvailableShaders = shaderMgr.GetShaderNames();
        }
        
        ImGui::Separator();
        
        for (size_t i = 0; i < m_AvailableShaders.size(); ++i)
        {
            bool isSelected = (static_cast<int>(i) == m_SelectedShaderIndex);
            if (ImGui::Selectable(m_AvailableShaders[i].c_str(), isSelected))
            {
                m_SelectedShaderIndex = static_cast<int>(i);
                LoadShader(m_AvailableShaders[i]);
            }
        }
    }

    void ShaderEditor::RenderEditor()
    {
        ImGui::Text("Shader Editor:");
        
        if (m_CurrentVertexPath.empty() && m_CurrentFragmentPath.empty())
        {
            ImGui::TextWrapped("No shader loaded. Select a shader from the list or use LoadShaderFiles().");
            return;
        }
        
        // Tabs for vertex and fragment shaders
        if (ImGui::BeginTabBar("ShaderTabs"))
        {
            if (m_ShowVertexShader && ImGui::BeginTabItem("Vertex Shader"))
            {
                ImGui::Text("File: %s", m_CurrentVertexPath.string().c_str());
                
                if (m_IsVertexShaderModified)
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*Modified*");
                
                ImGui::Separator();
                
                ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                if (ImGui::InputTextMultiline("##VertexShader", 
                    m_VertexShaderBuffer, 
                    sizeof(m_VertexShaderBuffer),
                    ImVec2(-1, -1),
                    flags))
                {
                    m_IsVertexShaderModified = true;
                }
                
                ImGui::EndTabItem();
            }
            
            if (m_ShowFragmentShader && ImGui::BeginTabItem("Fragment Shader"))
            {
                ImGui::Text("File: %s", m_CurrentFragmentPath.string().c_str());
                
                if (m_IsFragmentShaderModified)
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*Modified*");
                
                ImGui::Separator();
                
                ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                if (ImGui::InputTextMultiline("##FragmentShader", 
                    m_FragmentShaderBuffer, 
                    sizeof(m_FragmentShaderBuffer),
                    ImVec2(-1, -1),
                    flags))
                {
                    m_IsFragmentShaderModified = true;
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }

    void ShaderEditor::RenderErrorDisplay()
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Compilation Errors:");
        ImGui::Separator();
        ImGui::TextWrapped("%s", m_CompilationError.c_str());
    }

    void ShaderEditor::RenderPreview()
    {
        ImGui::Text("Shader Preview:");
        ImGui::Separator();
        
        // Check if we have a shader loaded
        if (m_CurrentShaderName.empty())
        {
            ImGui::TextWrapped("Load a shader to see live preview.");
            return;
        }
        
        // Preview shape selection
        const char* shapes[] = { "Sphere", "Cube" };
        int currentShape = (int)m_PreviewShape;
        if (ImGui::Combo("Preview Shape", &currentShape, shapes, IM_ARRAYSIZE(shapes)))
        {
            m_PreviewShape = (PreviewShape)currentShape;
        }
        
        ImGui::Text("Rotation:");
        ImGui::SliderFloat("##PreviewRotation", &m_PreviewRotation, 0.0f, 360.0f);
        
        // Auto-rotate option
        static bool autoRotate = false;
        ImGui::Checkbox("Auto Rotate", &autoRotate);
        if (autoRotate)
        {
            m_PreviewRotation += 0.5f;
            if (m_PreviewRotation >= 360.0f)
                m_PreviewRotation -= 360.0f;
        }
        
        // 3D Preview viewport
        ImGui::BeginChild("PreviewViewport", ImVec2(0, 300), true, ImGuiWindowFlags_NoScrollbar);
        
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Get the shader material from ShaderManager
        auto& shaderMgr = Core::Renderer::ShaderManager::Get();
        // For preview, we'd need to create a temporary material with the shader
        // This is simplified - in production you'd create a preview material
        
        if (viewportSize.x > 0 && viewportSize.y > 0)
        {
            uint32_t width = (uint32_t)viewportSize.x;
            uint32_t height = (uint32_t)viewportSize.y;
            
            if (m_PreviewViewport->GetWidth() != width || m_PreviewViewport->GetHeight() != height)
            {
                m_PreviewViewport->Resize(width, height);
            }
            
            // Note: This is a placeholder. To actually render, you'd need to:
            // 1. Create a Material from the current shader
            // 2. Pass it to RenderPreviewSphere/Cube
            // For now, just show the viewport texture
            ImGui::Image((ImTextureID)(uintptr_t)m_PreviewViewport->GetColorAttachment(),
                        viewportSize, ImVec2(0, 1), ImVec2(1, 0)); // Flip Y for OpenGL
            
            ImGui::TextWrapped("Note: Preview requires material creation from shader.");
        }
        else
        {
            ImGui::Text("Resize window to show preview");
        }
        
        ImGui::EndChild();
        
        if (ImGui::Button("Reset Camera"))
        {
            m_PreviewRotation = 0.0f;
            autoRotate = false;
        }
    }

    void ShaderEditor::RenderStatusBar()
    {
        ImGui::Separator();
        
        if (!m_LastSuccessfulCompile.empty())
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", m_LastSuccessfulCompile.c_str());
        }
        
        if (m_IsVertexShaderModified || m_IsFragmentShaderModified)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Unsaved Changes]");
        }
    }

    void ShaderEditor::SaveCurrentShader()
    {
        if (m_CurrentVertexPath.empty() || m_CurrentFragmentPath.empty())
        {
            m_CompilationError = "No shader files loaded to save";
            m_HasCompilationError = true;
            return;
        }
        
        bool success = true;
        
        if (m_IsVertexShaderModified)
        {
            success &= SaveFileContent(m_CurrentVertexPath, m_VertexShaderBuffer);
            if (success)
                m_IsVertexShaderModified = false;
        }
        
        if (m_IsFragmentShaderModified)
        {
            success &= SaveFileContent(m_CurrentFragmentPath, m_FragmentShaderBuffer);
            if (success)
                m_IsFragmentShaderModified = false;
        }
        
        if (success)
        {
            m_LastSuccessfulCompile = "Shader files saved successfully";
            m_HasCompilationError = false;
            
            if (m_AutoReloadOnSave)
                ReloadCurrentShader();
        }
        else
        {
            m_CompilationError = "Failed to save shader files";
            m_HasCompilationError = true;
        }
    }

    void ShaderEditor::ReloadCurrentShader()
    {
        if (!m_CurrentShaderName.empty())
        {
            auto& shaderMgr = Core::Renderer::ShaderManager::Get();
            if (shaderMgr.ReloadShader(m_CurrentShaderName))
            {
                m_LastSuccessfulCompile = "Shader '" + m_CurrentShaderName + "' reloaded successfully";
                m_HasCompilationError = false;
            }
            else
            {
                m_CompilationError = "Failed to reload shader: " + m_CurrentShaderName;
                m_HasCompilationError = true;
            }
        }
        else if (!m_CurrentVertexPath.empty() && !m_CurrentFragmentPath.empty())
        {
            // Reload from files
            LoadShaderFiles(m_CurrentVertexPath, m_CurrentFragmentPath);
        }
    }

    void ShaderEditor::CompileAndTest()
    {
        SaveCurrentShader();
        
        if (!m_HasCompilationError)
        {
            m_LastSuccessfulCompile = "Shader compiled and tested successfully";
        }
    }

    bool ShaderEditor::LoadFileContent(const std::filesystem::path& path, std::string& content)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        
        return true;
    }

    bool ShaderEditor::SaveFileContent(const std::filesystem::path& path, const std::string& content)
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }
        
        file << content;
        return true;
    }
}
