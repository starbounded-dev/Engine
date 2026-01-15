#include "ModelPanel.h"
#include "Core/Renderer/Camera.h"
#include "Core/Utilities/FileSystem.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Editor
{
    ModelPanel::ModelPanel()
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
        
        // Create viewport for live preview and set framebuffer
        m_Viewport = std::make_unique<Core::Editor::Viewport>("Model Preview");
        m_Viewport->SetFramebuffer(m_PreviewFramebuffer, 0);
        
        // Create a simple material for preview
        // Note: These paths should be adjusted to your actual shader paths
        m_PreviewMaterial = std::make_shared<Core::Renderer::Material>(
            "Resources/Shaders/DebugModel.vert.glsl",
            "Resources/Shaders/DebugModel.frag.glsl"
        );
        
        // Create UniformBuffer for per-object matrices (Model, View, Projection)
        // Binding point 1 = PerObject, size = 3 mat4 matrices (48 floats = 192 bytes)
        m_PerObjectUBO = std::make_shared<Core::Renderer::UniformBuffer>(
            3 * sizeof(glm::mat4),  // Model + View + Projection
            Core::Renderer::UBOBinding::PerObject,  // binding = 1
            true  // dynamic (updated every frame)
        );
        
        // Create preview meshes
        CreatePreviewMeshes();
    }
    
    void ModelPanel::CreatePreviewMeshes()
    {
        m_SphereMesh = CreateSphereMesh();
        m_CubeMesh = CreateCubeMesh();
    }
    
    std::unique_ptr<Core::Renderer::Mesh> ModelPanel::CreateSphereMesh()
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
    
    std::unique_ptr<Core::Renderer::Mesh> ModelPanel::CreateCubeMesh()
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

    void ModelPanel::OnImGuiRender()
    {
        if (!m_Enabled)
            return;

        ImGui::Begin("Model Viewer", &m_Enabled);

        // Tabs for different sections
        if (ImGui::BeginTabBar("ModelViewerTabs"))
        {
            if (ImGui::BeginTabItem("Model"))
            {
                RenderModelInfo();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Meshes"))
            {
                RenderMeshList();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Materials"))
            {
                RenderMaterialInfo();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Preview"))
            {
                RenderLivePreview();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Controls"))
            {
                RenderControls();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Statistics"))
            {
                RenderStatistics();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void ModelPanel::LoadModel(const std::string& path)
    {
        try
        {
            m_CurrentModel = std::make_shared<Core::Renderer::Model>(path);
            m_CurrentModelPath = path;
            m_SelectedMeshIndex = -1;
            
            // Add to recent models
            auto it = std::find(m_RecentModels.begin(), m_RecentModels.end(), path);
            if (it != m_RecentModels.end())
                m_RecentModels.erase(it);
            
            m_RecentModels.insert(m_RecentModels.begin(), path);
            if (m_RecentModels.size() > MaxRecentModels)
                m_RecentModels.resize(MaxRecentModels);
        }
        catch (const std::exception& e)
        {
            // Handle error (could add error message display)
            m_CurrentModel = nullptr;
            m_CurrentModelPath = "Error loading model: " + std::string(e.what());
        }
    }

    void ModelPanel::RenderModelInfo()
    {
        ImGui::Text("Load Model");
        ImGui::Separator();
        
        // Load button
        if (ImGui::Button("Load Model...", ImVec2(150, 0)))
        {
            OpenFileDialog();
        }
        
        ImGui::SameLine();
        ImGui::Text("Supported: .obj, .fbx, .gltf, .glb");
        
        // Recent models
        if (!m_RecentModels.empty())
        {
            ImGui::Spacing();
            ImGui::Text("Recent Models:");
            ImGui::Separator();
            
            for (size_t i = 0; i < m_RecentModels.size(); ++i)
            {
                std::string label = m_RecentModels[i];
                // Extract filename only for display
                size_t lastSlash = label.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                    label = label.substr(lastSlash + 1);
                
                if (ImGui::Selectable(label.c_str()))
                {
                    LoadModel(m_RecentModels[i]);
                }
                
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", m_RecentModels[i].c_str());
                    ImGui::EndTooltip();
                }
            }
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // Current model info
        if (m_CurrentModel)
        {
            ImGui::Text("Current Model:");
            ImGui::TextWrapped("%s", m_CurrentModelPath.c_str());
            
            ImGui::Spacing();
            ImGui::Text("Mesh Count: %zu", m_CurrentModel->GetMeshes().size());
            ImGui::Text("Material Count: %zu", m_CurrentModel->GetMaterials().size());
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No model loaded");
        }
    }

    void ModelPanel::RenderMeshList()
    {
        if (!m_CurrentModel)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No model loaded");
            return;
        }
        
        const auto& meshes = m_CurrentModel->GetMeshes();
        
        ImGui::Text("Meshes (%zu)", meshes.size());
        ImGui::Separator();
        
        for (size_t i = 0; i < meshes.size(); ++i)
        {
            const auto& mesh = meshes[i];
            
            ImGui::PushID(static_cast<int>(i));
            
            bool isSelected = (m_SelectedMeshIndex == static_cast<int>(i));
            if (ImGui::Selectable(("Mesh " + std::to_string(i)).c_str(), isSelected))
            {
                m_SelectedMeshIndex = static_cast<int>(i);
            }
            
            ImGui::Indent();
            ImGui::Text("Vertices: %zu", mesh.GetVertices().size());
            ImGui::Text("Indices: %zu", mesh.GetIndices().size());
            ImGui::Text("Triangles: %zu", mesh.GetIndices().size() / 3);
            ImGui::Text("Material Index: %u", mesh.GetMaterialIndex());
            ImGui::Unindent();
            
            ImGui::Spacing();
            ImGui::PopID();
        }
    }

    void ModelPanel::RenderMaterialInfo()
    {
        if (!m_CurrentModel)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No model loaded");
            return;
        }
        
        const auto& materials = m_CurrentModel->GetMaterials();
        
        ImGui::Text("Materials (%zu)", materials.size());
        ImGui::Separator();
        
        for (size_t i = 0; i < materials.size(); ++i)
        {
            const auto& mat = materials[i];
            
            if (ImGui::TreeNode(("Material " + std::to_string(i) + ": " + mat.Name).c_str()))
            {
                ImGui::Text("Name: %s", mat.Name.c_str());
                
                if (!mat.AlbedoPath.empty())
                    ImGui::Text("Albedo: %s", mat.AlbedoPath.c_str());
                
                if (!mat.NormalPath.empty())
                    ImGui::Text("Normal: %s", mat.NormalPath.c_str());
                
                if (!mat.MetallicRoughnessPath.empty())
                    ImGui::Text("Metallic/Roughness: %s", mat.MetallicRoughnessPath.c_str());
                
                if (!mat.EmissivePath.empty())
                    ImGui::Text("Emissive: %s", mat.EmissivePath.c_str());
                
                ImGui::TreePop();
            }
        }
    }

    void ModelPanel::RenderLivePreview()
    {
        ImGui::Checkbox("Enable Live Preview", &m_LivePreview);
        
        if (!m_LivePreview)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Live preview disabled");
            return;
        }
        
        ImGui::Separator();
        
        // Preview settings
        const char* shapeNames[] = { "Sphere", "Cube", "Loaded Model" };
        int currentShape = static_cast<int>(m_PreviewShape);
        if (ImGui::Combo("Preview Shape", &currentShape, shapeNames, IM_ARRAYSIZE(shapeNames)))
        {
            m_PreviewShape = static_cast<PreviewShape>(currentShape);
        }
        
        ImGui::Checkbox("Auto Rotate", &m_AutoRotate);
        
        if (m_AutoRotate)
        {
            m_ModelRotation += 0.5f;
            if (m_ModelRotation >= 360.0f)
                m_ModelRotation -= 360.0f;
        }
        else
        {
            ImGui::SliderFloat("Rotation", &m_ModelRotation, 0.0f, 360.0f);
        }
        
        ImGui::Separator();
        
        // Render the preview
        if (m_PreviewFramebuffer && m_PreviewMaterial)
        {
            // Get available size for preview
            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            int width = static_cast<int>(availableSize.x);
            int height = static_cast<int>(availableSize.y - 50); // Leave space for controls
            
            if (width > 0 && height > 0)
            {
                // Resize framebuffer if needed
                if (m_PreviewFramebuffer->GetSpec().Width != static_cast<uint32_t>(width) || 
                    m_PreviewFramebuffer->GetSpec().Height != static_cast<uint32_t>(height))
                {
                    m_PreviewFramebuffer->Resize(width, height);
                }
                
                // Render to framebuffer
                m_PreviewFramebuffer->Bind();
                
                glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                glEnable(GL_DEPTH_TEST);
                
                // Set up camera
                Core::Renderer::Camera camera;
                camera.SetProjectionType(Core::Renderer::ProjectionType::Perspective);
                camera.SetPerspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
                
                // Calculate camera position based on controls
                float camX = m_CameraDistance * cos(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
                float camY = m_CameraDistance * sin(glm::radians(m_CameraPitch));
                float camZ = m_CameraDistance * sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch));
                
                camera.SetPosition(glm::vec3(camX, camY, camZ) + m_ModelOffset);
                camera.LookAt(m_ModelOffset);
                
                // Create model matrix
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, m_ModelOffset);
                model = glm::rotate(model, glm::radians(m_ModelRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(m_ModelScale));
                
                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 projection = camera.GetProjectionMatrix();
                
                // Upload matrices to UniformBuffer (Model at offset 0, View at 64, Projection at 128)
                m_PerObjectUBO->SetData(&model, sizeof(glm::mat4), 0);
                m_PerObjectUBO->SetData(&view, sizeof(glm::mat4), sizeof(glm::mat4));
                m_PerObjectUBO->SetData(&projection, sizeof(glm::mat4), 2 * sizeof(glm::mat4));
                m_PerObjectUBO->BindBase();
                
                // Bind material (shaders expect matrices from UBO)
                m_PreviewMaterial->Bind();
                
                // Draw the model or preview shape
                if (m_PreviewShape == PreviewShape::LoadedModel && m_CurrentModel)
                {
                    m_CurrentModel->Draw();
                }
                else if (m_PreviewShape == PreviewShape::Sphere && m_SphereMesh)
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
                ImGui::Image(
                    imguiTex,
                    ImVec2(static_cast<float>(width), static_cast<float>(height)),
                    ImVec2(0, 1), // Flip Y for OpenGL
                    ImVec2(1, 0)
                );
            }
        }
    }

    void ModelPanel::RenderControls()
    {
        ImGui::Text("Camera Controls");
        ImGui::Separator();
        
        ImGui::SliderFloat("Distance", &m_CameraDistance, 1.0f, 20.0f);
        ImGui::SliderFloat("Yaw", &m_CameraYaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Pitch", &m_CameraPitch, -89.0f, 89.0f);
        
        if (ImGui::Button("Reset Camera"))
        {
            m_CameraDistance = 5.0f;
            m_CameraYaw = 0.0f;
            m_CameraPitch = 0.0f;
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Model Transform");
        ImGui::Separator();
        
        ImGui::SliderFloat("Scale", &m_ModelScale, 0.1f, 10.0f);
        ImGui::DragFloat3("Offset", glm::value_ptr(m_ModelOffset), 0.01f);
        
        if (ImGui::Button("Reset Transform"))
        {
            m_ModelScale = 1.0f;
            m_ModelOffset = glm::vec3(0.0f);
            m_ModelRotation = 0.0f;
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        ImGui::Text("Display Options");
        ImGui::Separator();
        
        ImGui::Checkbox("Show Wireframe", &m_ShowWireframe);
        ImGui::Checkbox("Show Normals", &m_ShowNormals);
        ImGui::Checkbox("Show Bounding Box", &m_ShowBoundingBox);
    }

    void ModelPanel::RenderStatistics()
    {
        if (!m_CurrentModel)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No model loaded");
            return;
        }
        
        const auto& meshes = m_CurrentModel->GetMeshes();
        
        // Calculate totals
        size_t totalVertices = 0;
        size_t totalIndices = 0;
        size_t totalTriangles = 0;
        
        for (const auto& mesh : meshes)
        {
            totalVertices += mesh.GetVertices().size();
            totalIndices += mesh.GetIndices().size();
            totalTriangles += mesh.GetIndices().size() / 3;
        }
        
        ImGui::Text("Model Statistics");
        ImGui::Separator();
        
        ImGui::Text("Total Meshes: %zu", meshes.size());
        ImGui::Text("Total Vertices: %zu", totalVertices);
        ImGui::Text("Total Indices: %zu", totalIndices);
        ImGui::Text("Total Triangles: %zu", totalTriangles);
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // Memory estimation
        size_t vertexMemory = totalVertices * sizeof(Core::Renderer::MeshVertex);
        size_t indexMemory = totalIndices * sizeof(uint32_t);
        size_t totalMemory = vertexMemory + indexMemory;
        
        ImGui::Text("Memory Usage (Estimated)");
        ImGui::Separator();
        
        ImGui::Text("Vertex Data: %.2f KB", vertexMemory / 1024.0f);
        ImGui::Text("Index Data: %.2f KB", indexMemory / 1024.0f);
        ImGui::Text("Total: %.2f KB", totalMemory / 1024.0f);
        
        if (m_SelectedMeshIndex >= 0 && m_SelectedMeshIndex < static_cast<int>(meshes.size()))
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Selected Mesh Statistics");
            ImGui::Separator();
            
            const auto& mesh = meshes[m_SelectedMeshIndex];
            ImGui::Text("Mesh Index: %d", m_SelectedMeshIndex);
            ImGui::Text("Vertices: %zu", mesh.GetVertices().size());
            ImGui::Text("Indices: %zu", mesh.GetIndices().size());
            ImGui::Text("Triangles: %zu", mesh.GetIndices().size() / 3);
            ImGui::Text("Material Index: %u", mesh.GetMaterialIndex());
        }
    }

    void ModelPanel::OpenFileDialog()
    {
        // Use FileSystem utility to open native file dialog
        auto modelPath = Core::Utilities::FileSystem::OpenFileDialog(
            Core::Utilities::FileSystem::FILTER_MODELS
        );

        if (modelPath.has_value())
        {
            LoadModel(modelPath.value());
        }

        // Declare pathBuffer to fix the error
        char pathBuffer[1024] = { '\0' };

        if (ImGui::Button("Load", ImVec2(120, 0)))
        {
            if (pathBuffer[0] != '\0')
            {
                LoadModel(pathBuffer);
                pathBuffer[0] = '\0';
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            pathBuffer[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
