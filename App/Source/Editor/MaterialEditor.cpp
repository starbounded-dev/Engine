#include "MaterialEditor.h"
#include "Core/Utilities/FileSystem.h"
#include "Core/Renderer/Camera.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstring>

namespace Editor
{
    MaterialEditor::MaterialEditor()
    {
        // Create preview framebuffer
        Core::Renderer::FramebufferSpec fbSpec;
        fbSpec.Width = 512;
        fbSpec.Height = 512;
        fbSpec.Attachments = {
            {Core::Renderer::FramebufferTextureFormat::RGBA8 },         // Color
            {Core::Renderer::FramebufferTextureFormat::Depth24Stencil8 } // Depth/Stencil
        };
        m_PreviewFramebuffer = std::make_shared<Core::Renderer::Framebuffer>(fbSpec);
        
        // Create preview viewport and set framebuffer
        m_PreviewViewport = std::make_unique<Core::Editor::Viewport>("Material Preview");
        m_PreviewViewport->SetFramebuffer(m_PreviewFramebuffer, 0);
        
        // Create UniformBuffer for per-object matrices (Model, View, Projection)
        m_PerObjectUBO = std::make_shared<Core::Renderer::UniformBuffer>(
            3 * sizeof(glm::mat4),  // Model + View + Projection
            Core::Renderer::UBOBinding::PerObject,  // binding = 1
            true  // dynamic (updated every frame)
        );
        
        // Add some default templates
        MaterialTemplate unlitTemplate;
        unlitTemplate.Name = "Unlit";
        unlitTemplate.VertexShader = "Resources/Shaders/Unlit.vert.glsl";
        unlitTemplate.FragmentShader = "Resources/Shaders/Unlit.frag.glsl";
        unlitTemplate.DefaultValues["u_Color"] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        unlitTemplate.TextureSlots = { "u_Texture" };
        m_Templates.push_back(unlitTemplate);
        
        MaterialTemplate pbrTemplate;
        pbrTemplate.Name = "PBR";
        pbrTemplate.VertexShader = "Resources/Shaders/PBR.vert.glsl";
        pbrTemplate.FragmentShader = "Resources/Shaders/PBR.frag.glsl";
        pbrTemplate.DefaultValues["u_Albedo"] = glm::vec3(1.0f, 1.0f, 1.0f);
        pbrTemplate.DefaultValues["u_Metallic"] = 0.5f;
        pbrTemplate.DefaultValues["u_Roughness"] = 0.5f;
        pbrTemplate.DefaultValues["u_AO"] = 1.0f;
        pbrTemplate.TextureSlots = { "u_AlbedoMap", "u_NormalMap", "u_MetallicMap", "u_RoughnessMap", "u_AOMap" };
        m_Templates.push_back(pbrTemplate);
        
        MaterialTemplate standardTemplate;
        standardTemplate.Name = "Standard";
        standardTemplate.VertexShader = "Resources/Shaders/Standart.vert.glsl";
        standardTemplate.FragmentShader = "Resources/Shaders/Standart.frag.glsl";
        standardTemplate.DefaultValues["u_Diffuse"] = glm::vec3(0.8f, 0.8f, 0.8f);
        standardTemplate.DefaultValues["u_Specular"] = glm::vec3(1.0f, 1.0f, 1.0f);
        standardTemplate.DefaultValues["u_Shininess"] = 32.0f;
        standardTemplate.TextureSlots = { "u_DiffuseMap", "u_SpecularMap", "u_NormalMap" };
        m_Templates.push_back(standardTemplate);
        
        // Create preview meshes
        CreatePreviewMeshes();
    }
    
    void MaterialEditor::CreatePreviewMeshes()
    {
        m_SphereMesh = CreateSphereMesh();
        m_CubeMesh = CreateCubeMesh();
    }
    
    std::unique_ptr<Core::Renderer::Mesh> MaterialEditor::CreateSphereMesh()
    {
        std::vector<Core::Renderer::MeshVertex> vertices;
        std::vector<uint32_t> indices;
        
        // Generate UV sphere with 32 segments and 16 rings
        const int segments = 32;
        const int rings = 16;
        const float radius = 1.0f;
        
        // Generate vertices
        for (int ring = 0; ring <= rings; ++ring)
        {
            float phi = glm::pi<float>() * float(ring) / float(rings);
            for (int seg = 0; seg <= segments; ++seg)
            {
                float theta = 2.0f * glm::pi<float>() * float(seg) / float(segments);
                
                Core::Renderer::MeshVertex vertex;
                vertex.Position.x = radius * sin(phi) * cos(theta);
                vertex.Position.y = radius * cos(phi);
                vertex.Position.z = radius * sin(phi) * sin(theta);
                vertex.Normal = glm::normalize(vertex.Position);
                vertex.TexCoord.x = float(seg) / float(segments);
                vertex.TexCoord.y = float(ring) / float(rings);
                
                vertices.push_back(vertex);
            }
        }
        
        // Generate indices
        for (int ring = 0; ring < rings; ++ring)
        {
            for (int seg = 0; seg < segments; ++seg)
            {
                int current = ring * (segments + 1) + seg;
                int next = current + segments + 1;
                
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);
                
                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
        
        return std::make_unique<Core::Renderer::Mesh>(vertices, indices, 0);
    }
    
    std::unique_ptr<Core::Renderer::Mesh> MaterialEditor::CreateCubeMesh()
    {
        std::vector<Core::Renderer::MeshVertex> vertices;
        std::vector<uint32_t> indices;
        
        // Cube vertices with normals and UVs
        const float size = 1.0f;
        
        // Front face
        vertices.push_back({{-size, -size,  size}, { 0, 0, 1}, {0, 0}});
        vertices.push_back({{ size, -size,  size}, { 0, 0, 1}, {1, 0}});
        vertices.push_back({{ size,  size,  size}, { 0, 0, 1}, {1, 1}});
        vertices.push_back({{-size,  size,  size}, { 0, 0, 1}, {0, 1}});
        
        // Back face
        vertices.push_back({{ size, -size, -size}, { 0, 0,-1}, {0, 0}});
        vertices.push_back({{-size, -size, -size}, { 0, 0,-1}, {1, 0}});
        vertices.push_back({{-size,  size, -size}, { 0, 0,-1}, {1, 1}});
        vertices.push_back({{ size,  size, -size}, { 0, 0,-1}, {0, 1}});
        
        // Left face
        vertices.push_back({{-size, -size, -size}, {-1, 0, 0}, {0, 0}});
        vertices.push_back({{-size, -size,  size}, {-1, 0, 0}, {1, 0}});
        vertices.push_back({{-size,  size,  size}, {-1, 0, 0}, {1, 1}});
        vertices.push_back({{-size,  size, -size}, {-1, 0, 0}, {0, 1}});
        
        // Right face
        vertices.push_back({{ size, -size,  size}, { 1, 0, 0}, {0, 0}});
        vertices.push_back({{ size, -size, -size}, { 1, 0, 0}, {1, 0}});
        vertices.push_back({{ size,  size, -size}, { 1, 0, 0}, {1, 1}});
        vertices.push_back({{ size,  size,  size}, { 1, 0, 0}, {0, 1}});
        
        // Top face
        vertices.push_back({{-size,  size,  size}, { 0, 1, 0}, {0, 0}});
        vertices.push_back({{ size,  size,  size}, { 0, 1, 0}, {1, 0}});
        vertices.push_back({{ size,  size, -size}, { 0, 1, 0}, {1, 1}});
        vertices.push_back({{-size,  size, -size}, { 0, 1, 0}, {0, 1}});
        
        // Bottom face
        vertices.push_back({{-size, -size, -size}, { 0,-1, 0}, {0, 0}});
        vertices.push_back({{ size, -size, -size}, { 0,-1, 0}, {1, 0}});
        vertices.push_back({{ size, -size,  size}, { 0,-1, 0}, {1, 1}});
        vertices.push_back({{-size, -size,  size}, { 0,-1, 0}, {0, 1}});
        
        // Indices (6 faces × 2 triangles × 3 vertices)
        for (uint32_t i = 0; i < 6; ++i)
        {
            uint32_t base = i * 4;
            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
        
        return std::make_unique<Core::Renderer::Mesh>(vertices, indices, 0);
    }

    void MaterialEditor::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        ImGui::Begin("Material Editor", &m_Enabled);

        // Tabs for different sections
        if (ImGui::BeginTabBar("MaterialEditorTabs"))
        {
            if (ImGui::BeginTabItem("Properties"))
            {
                RenderPropertyEditor();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Textures"))
            {
                RenderTextureSlots();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Preview"))
            {
                RenderLivePreview();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Actions"))
            {
                RenderActions();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Templates"))
            {
                RenderTemplateSelector();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

        standardTemplate.TextureSlots = { "u_DiffuseMap", "u_SpecularMap", "u_NormalMap" };
        m_Templates.push_back(standardTemplate);
    }

    void MaterialEditor::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        ImGui::Begin("Material Editor", &m_Enabled);

        // Toolbar
        if (ImGui::Button("New Material"))
            m_ShowMaterialCreator = true;
        
        ImGui::SameLine();
        if (ImGui::Button("Load Material"))
            LoadMaterial();
        
        ImGui::SameLine();
        if (ImGui::Button("Save Material"))
            SaveMaterial();
        
        ImGui::SameLine();
        ImGui::Checkbox("Live Preview", &m_LivePreview);

        ImGui::Separator();

        // Main content
        if (m_CurrentMaterial)
        {
            ImGui::BeginChild("MaterialEditorContent", ImVec2(0, 0), true);
            
            // Material name/info
            ImGui::Text("Current Material");
            ImGui::Separator();
            
            // Tabbed interface
            if (ImGui::BeginTabBar("MaterialEditorTabs"))
            {
                if (ImGui::BeginTabItem("Properties"))
                {
                    RenderPropertyEditor();
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Textures"))
                {
                    RenderTextureSlots();
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Preview"))
                {
                    RenderLivePreview();
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Actions"))
                {
                    RenderActions();
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }
            
            ImGui::EndChild();
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No material selected");
            ImGui::Text("Create a new material or load an existing one");
            
            ImGui::Separator();
            RenderTemplateSelector();
        }

        ImGui::End();

        // Material creator popup
        if (m_ShowMaterialCreator)
        {
            ImGui::OpenPopup("Create Material");
            m_ShowMaterialCreator = false;
        }

        if (ImGui::BeginPopupModal("Create Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Create a new material from a template");
            ImGui::Separator();
            
            ImGui::InputText("Material Name", m_NewMaterialName, sizeof(m_NewMaterialName));
            
            ImGui::Text("Select Template:");
            for (size_t i = 0; i < m_Templates.size(); ++i)
            {
                if (ImGui::Selectable(m_Templates[i].Name.c_str(), m_SelectedTemplateIndex == (int)i))
                    m_SelectedTemplateIndex = (int)i;
            }
            
            ImGui::Separator();
            
            if (ImGui::Button("Create"))
            {
                if (m_SelectedTemplateIndex >= 0 && m_SelectedTemplateIndex < (int)m_Templates.size())
                {
                    CreateMaterialFromTemplate(m_Templates[m_SelectedTemplateIndex]);
                }
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }

    void MaterialEditor::SetMaterial(std::shared_ptr<Core::Renderer::Material> material)
    {
        m_CurrentMaterial = material;
        
        // Update texture slots
        m_TextureSlots.clear();
        if (material)
        {
            const auto& textures = material->GetTextures();
            for (const auto& tex : textures)
            {
                TextureSlotUI slot;
                slot.SlotName = tex.Uniform;
                slot.TextureUnit = tex.Slot;
                slot.CurrentTextureID = tex.TextureID;
                m_TextureSlots.push_back(slot);
            }
        }
    }

    void MaterialEditor::AddTemplate(const MaterialTemplate& tmpl)
    {
        m_Templates.push_back(tmpl);
    }

    void MaterialEditor::RenderMaterialSelector()
    {
        ImGui::Text("Material Library");
        ImGui::Separator();
        
        for (size_t i = 0; i < m_MaterialLibrary.size(); ++i)
        {
            std::string label = "Material " + std::to_string(i);
            if (ImGui::Selectable(label.c_str(), m_SelectedMaterialIndex == (int)i))
            {
                m_SelectedMaterialIndex = (int)i;
                SetMaterial(m_MaterialLibrary[i]);
            }
        }
    }

    void MaterialEditor::RenderPropertyEditor()
    {
        if (!m_CurrentMaterial)
            return;

        ImGui::Text("Material Properties");
        ImGui::Separator();
        
        // Get current values from material
        auto& values = const_cast<std::unordered_map<std::string, Core::Renderer::MaterialValue>&>(
            m_CurrentMaterial->GetValues()
        );
        
        // Render each property
        for (auto& [name, value] : values)
        {
            ImGui::PushID(name.c_str());
            RenderMaterialValueEditor(name, value);
            ImGui::PopID();
        }
        
        // Apply changes immediately if live preview is enabled
        if (m_LivePreview)
        {
            // Material automatically updates when we modify the values
        }
    }

    void MaterialEditor::RenderTextureSlots()
    {
        if (!m_CurrentMaterial)
            return;

        ImGui::Text("Texture Slots");
        ImGui::Separator();
        
        for (size_t i = 0; i < m_TextureSlots.size(); ++i)
        {
            auto& slot = m_TextureSlots[i];
            
            ImGui::PushID((int)i);
            
            ImGui::Text("Slot: %s (Unit %u)", slot.SlotName.c_str(), slot.TextureUnit);
            ImGui::Text("Texture ID: %u", slot.CurrentTextureID);
            
            ImGui::InputText("File Path", slot.FilePath, sizeof(slot.FilePath));
            
            ImGui::SameLine();
            if (ImGui::Button("Load"))
            {
                // Use FileSystem utility to open image file dialog
                auto texturePath = Core::Utilities::FileSystem::OpenFileDialog(
                    Core::Utilities::FileSystem::FILTER_IMAGES
                );
                
                if (texturePath.has_value())
                {
                    // Copy path to buffer
                    strncpy(slot.FilePath, texturePath.value().c_str(), sizeof(slot.FilePath) - 1);
                    slot.FilePath[sizeof(slot.FilePath) - 1] = '\0';
                    
                    // TODO: Actually load the texture from file
                    // For now, this is a placeholder
                    ImGui::OpenPopup("Texture Load Info");
                }
            }
            
            if (ImGui::BeginPopup("Texture Load Info"))
            {
                ImGui::Text("Texture loading not yet fully implemented");
                ImGui::Text("Selected file: %s", slot.FilePath);
                ImGui::EndPopup();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                slot.CurrentTextureID = 0;
                m_CurrentMaterial->SetTexture(slot.SlotName, slot.TextureUnit, 0);
            }
            
            ImGui::Separator();
            ImGui::PopID();
        }
        
        // Add new texture slot
        if (ImGui::Button("Add Texture Slot"))
        {
            TextureSlotUI newSlot;
            newSlot.SlotName = "u_NewTexture";
            newSlot.TextureUnit = (uint32_t)m_TextureSlots.size();
            m_TextureSlots.push_back(newSlot);
        }
    }

    void MaterialEditor::RenderTemplateSelector()
    {
        ImGui::Text("Material Templates");
        ImGui::Separator();
        
        for (size_t i = 0; i < m_Templates.size(); ++i)
        {
            const auto& tmpl = m_Templates[i];
            
            ImGui::PushID((int)i);
            
            if (ImGui::Button(tmpl.Name.c_str(), ImVec2(120, 40)))
            {
                CreateMaterialFromTemplate(tmpl);
            }
            
            if ((i + 1) % 3 != 0)
                ImGui::SameLine();
            
            ImGui::PopID();
        }
    }

    void MaterialEditor::RenderLivePreview()
    {
        if (!m_CurrentMaterial)
            return;
            
        ImGui::Text("Live Preview");
        ImGui::Separator();
        
        // Preview shape selection
        const char* shapes[] = { "Sphere", "Cube" };
        int currentShape = (int)m_PreviewShape;
        if (ImGui::Combo("Preview Shape", &currentShape, shapes, IM_ARRAYSIZE(shapes)))
        {
            m_PreviewShape = (PreviewShape)currentShape;
        }
        
        ImGui::Text("Preview Rotation:");
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
        ImGui::BeginChild("PreviewViewport", ImVec2(0, 400), true, ImGuiWindowFlags_NoScrollbar);
        
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Resize framebuffer if needed
        if (viewportSize.x > 0 && viewportSize.y > 0)
        {
            uint32_t width = (uint32_t)viewportSize.x;
            uint32_t height = (uint32_t)viewportSize.y;
            
            // Resize framebuffer
            if (m_PreviewFramebuffer->GetSpec().Width != width || 
                m_PreviewFramebuffer->GetSpec().Height != height)
            {
                m_PreviewFramebuffer->Resize(width, height);
            }
            
            // Render to framebuffer
            m_PreviewFramebuffer->Bind();
            glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            
            // Set up simple camera for preview
            Core::Renderer::Camera camera;
            camera.SetProjectionType(Core::Renderer::ProjectionType::Perspective);
            camera.SetPerspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
            camera.SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
            camera.LookAt(glm::vec3(0.0f));
            
            // Create model matrix with rotation
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(m_PreviewRotation), glm::vec3(0.0f, 1.0f, 0.0f));
            
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = camera.GetProjectionMatrix();
            
            // Upload matrices to UniformBuffer (Model at offset 0, View at 64, Projection at 128)
            m_PerObjectUBO->SetData(&model, sizeof(glm::mat4), 0);
            m_PerObjectUBO->SetData(&view, sizeof(glm::mat4), sizeof(glm::mat4));
            m_PerObjectUBO->SetData(&projection, sizeof(glm::mat4), 2 * sizeof(glm::mat4));
            m_PerObjectUBO->BindBase();
            
            // Bind material (shaders expect matrices from UBO)
            m_CurrentMaterial->Bind();
            
            // Draw preview geometry (sphere or cube)
            if (m_PreviewShape == PreviewShape::Sphere && m_SphereMesh)
            {
                m_SphereMesh->Draw();
            }
            else if (m_PreviewShape == PreviewShape::Cube && m_CubeMesh)
            {
                m_CubeMesh->Draw();
            }
            
            glUseProgram(0);
            Core::Renderer::Framebuffer::Unbind();
            
            // Display the rendered texture
            uint32_t textureID = m_PreviewFramebuffer->GetColorAttachmentID(0);
            ImTextureID imguiTex = (ImTextureID)(intptr_t)textureID;
            ImGui::Image(imguiTex, viewportSize, ImVec2(0, 1), ImVec2(1, 0)); // Flip Y for OpenGL
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

    void MaterialEditor::RenderActions()
    {
        ImGui::Text("Material Actions");
        ImGui::Separator();
        
        if (ImGui::Button("Rebuild Shader", ImVec2(-1, 0)))
        {
            if (m_CurrentMaterial)
            {
                m_CurrentMaterial->Rebuild();
                ImGui::OpenPopup("Rebuild Status");
            }
        }
        
        if (ImGui::BeginPopup("Rebuild Status"))
        {
            ImGui::Text("Shader rebuilt successfully!");
            ImGui::EndPopup();
        }
        
        if (ImGui::Button("Edit Shaders", ImVec2(-1, 0)))
        {
            if (m_CurrentMaterial)
            {
                m_CurrentMaterial->LoadIntoShaderEditor();
            }
        }
        
        if (ImGui::Button("Clone Material", ImVec2(-1, 0)))
        {
            if (m_CurrentMaterial)
            {
                // TODO: Implement material cloning
                ImGui::OpenPopup("Clone Material");
            }
        }
        
        if (ImGui::BeginPopup("Clone Material"))
        {
            ImGui::Text("Material cloning not yet implemented");
            ImGui::EndPopup();
        }
        
        ImGui::Separator();
        
        ImGui::Text("Shader Paths:");
        if (m_CurrentMaterial)
        {
            ImGui::TextWrapped("Vertex: %s", m_CurrentMaterial->GetVertexPath().c_str());
            ImGui::TextWrapped("Fragment: %s", m_CurrentMaterial->GetFragmentPath().c_str());
        }
    }

    void MaterialEditor::CreateMaterialFromTemplate(const MaterialTemplate& tmpl)
    {
        auto material = std::make_shared<Core::Renderer::Material>(
            tmpl.VertexShader,
            tmpl.FragmentShader
        );
        
        // Apply default values
        for (const auto& [name, value] : tmpl.DefaultValues)
        {
            // Use std::visit to apply the correct setter based on type
            std::visit([&](auto&& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, float>)
                    material->SetFloat(name, val);
                else if constexpr (std::is_same_v<T, int32_t>)
                    material->SetInt(name, val);
                else if constexpr (std::is_same_v<T, uint32_t>)
                    material->SetUInt(name, val);
                else if constexpr (std::is_same_v<T, glm::vec2>)
                    material->SetVec2(name, val);
                else if constexpr (std::is_same_v<T, glm::vec3>)
                    material->SetVec3(name, val);
                else if constexpr (std::is_same_v<T, glm::vec4>)
                    material->SetVec4(name, val);
                else if constexpr (std::is_same_v<T, glm::mat3>)
                    material->SetMat3(name, val);
                else if constexpr (std::is_same_v<T, glm::mat4>)
                    material->SetMat4(name, val);
            }, value);
        }
        
        // Add to library and set as current
        m_MaterialLibrary.push_back(material);
        SetMaterial(material);
    }

    void MaterialEditor::SaveMaterial()
    {
        if (!m_CurrentMaterial)
            return;
        
        // Use FileSystem utility to open save dialog
        auto savePath = Core::Utilities::FileSystem::SaveFileDialog(
            Core::Utilities::FileSystem::FILTER_MATERIALS
        );
        
        if (savePath.has_value())
        {
            // TODO: Implement material serialization to file
            // For now, just show success message
            ImGui::OpenPopup("Save Material Success");
        }
    }

    void MaterialEditor::LoadMaterial()
    {
        // Use FileSystem utility to open file dialog
        auto loadPath = Core::Utilities::FileSystem::OpenFileDialog(
            Core::Utilities::FileSystem::FILTER_MATERIALS
        );
        
        if (loadPath.has_value())
        {
            // TODO: Implement material deserialization from file
            // For now, just show the path
            ImGui::OpenPopup("Load Material");
        }
    }

    void MaterialEditor::RenderMaterialValueEditor(const std::string& name, Core::Renderer::MaterialValue& value)
    {
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            
            if constexpr (std::is_same_v<T, float>)
            {
                if (ImGui::DragFloat(name.c_str(), &val, 0.01f))
                    m_CurrentMaterial->SetFloat(name, val);
            }
            else if constexpr (std::is_same_v<T, int32_t>)
            {
                if (ImGui::DragInt(name.c_str(), &val))
                    m_CurrentMaterial->SetInt(name, val);
            }
            else if constexpr (std::is_same_v<T, uint32_t>)
            {
                int temp = (int)val;
                if (ImGui::DragInt(name.c_str(), &temp, 1.0f, 0))
                {
                    val = (uint32_t)temp;
                    m_CurrentMaterial->SetUInt(name, val);
                }
            }
            else if constexpr (std::is_same_v<T, glm::vec2>)
            {
                if (ImGui::DragFloat2(name.c_str(), &val.x, 0.01f))
                    m_CurrentMaterial->SetVec2(name, val);
            }
            else if constexpr (std::is_same_v<T, glm::vec3>)
            {
                if (ImGui::ColorEdit3(name.c_str(), &val.x) || ImGui::DragFloat3(name.c_str(), &val.x, 0.01f))
                    m_CurrentMaterial->SetVec3(name, val);
            }
            else if constexpr (std::is_same_v<T, glm::vec4>)
            {
                if (ImGui::ColorEdit4(name.c_str(), &val.x) || ImGui::DragFloat4(name.c_str(), &val.x, 0.01f))
                    m_CurrentMaterial->SetVec4(name, val);
            }
            else if constexpr (std::is_same_v<T, glm::mat3>)
            {
                ImGui::Text("%s (mat3)", name.c_str());
                // Matrix editing would require more complex UI
            }
            else if constexpr (std::is_same_v<T, glm::mat4>)
            {
                ImGui::Text("%s (mat4)", name.c_str());
                // Matrix editing would require more complex UI
            }
        }, value);
    }
}
