
#include "Core/Layer.h"
#include "Core/Application.h"
#include "Core/Input/Input.h"

#include "Core/Renderer/Camera.h"
#include "Core/Renderer/Model.h"
#include "Core/Renderer/Material.h"
#include "Core/Renderer/UniformBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <filesystem>
#include <memory>

namespace Sandbox
{
    class SponzaTestLayer : public Core::Layer
    {
    public:
        SponzaTestLayer()
            : Core::Layer("SponzaTestLayer")
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            // ---- camera ----
            auto fb = Core::Application::Get().GetFramebufferSize();
            float aspect = (fb.y > 0.0f) ? (fb.x / fb.y) : 16.0f / 9.0f;
            m_Camera.SetPerspective(60.0f, aspect, 0.1f, 5000.0f);

            // ---- debug material (normal/UV shading) ----
            m_DebugMat = Core::Renderer::Material(
                "Resources/Shaders/DebugModel.vert.glsl",
                "Resources/Shaders/DebugModel.frag.glsl"
            );

            // ---- UBOs from shader ----
            {
                GLuint prog = m_DebugMat.GetProgram();
                auto frameLayout = Core::Renderer::UniformBufferLayout::Reflect(prog, "FrameData");
                auto objLayout   = Core::Renderer::UniformBufferLayout::Reflect(prog, "ObjectData");

                if (frameLayout.GetSize() > 0) m_FrameUBO  = Core::Renderer::UniformBuffer(frameLayout, 0, true);
                if (objLayout.GetSize() > 0)   m_ObjectUBO = Core::Renderer::UniformBuffer(objLayout, 1, true);
            }

            // ---- load sponza ----
            // Change this path to your sponza file:
            // Examples:
            //  - "Resources/Models/Sponza/sponza.obj"
            //  - "Resources/Models/Sponza/Sponza.gltf"
            m_SponzaPath = "Resources/Models/Sponza/Sponza.gltf";

            if (std::filesystem::exists(m_SponzaPath))
                m_Model = Core::Renderer::Model::LoadCached(m_SponzaPath, true);
        }

        void OnUpdate(float ts) override
        {
            m_Time += ts;

            if (Core::Input::IsKeyPressed(Core::Key::Escape))
                Core::Application::Get().Stop();

            auto fb = Core::Application::Get().GetFramebufferSize();
            if (fb.x != m_LastFB.x || fb.y != m_LastFB.y)
            {
                m_LastFB = fb;
                m_Camera.SetViewportSize(fb.x, fb.y);
            }

            if (!m_PauseOrbit)
                m_OrbitAngle += ts * m_OrbitSpeed;
        }

        void OnRender() override
        {
            auto fb = Core::Application::Get().GetFramebufferSize();
            glViewport(0, 0, (int)fb.x, (int)fb.y);
            glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (!m_Model)
                return;

            // ---- orbit camera around sponza ----
            const glm::vec3 target = m_SponzaTarget;
            const float r = m_OrbitRadius;

            glm::vec3 camPos;
            camPos.x = target.x + cosf(m_OrbitAngle) * r;
            camPos.z = target.z + sinf(m_OrbitAngle) * r;
            camPos.y = target.y + m_OrbitHeight;

            m_Camera.SetPosition(camPos);
            m_Camera.LookAt(target);

            // ---- frame UBO (binding 0) ----
            if (m_FrameUBO.GetRendererID() != 0)
            {
                const glm::mat4 vp = m_Camera.GetViewProjectionMatrix();

                // supports either "u_ViewProjection" or "FrameData.u_ViewProjection"
                if (m_FrameUBO.Has("u_ViewProjection"))
                    m_FrameUBO.SetMat4("u_ViewProjection", &vp[0][0], false);
                else if (m_FrameUBO.Has("FrameData.u_ViewProjection"))
                    m_FrameUBO.SetMat4("FrameData.u_ViewProjection", &vp[0][0], false);

                m_FrameUBO.Upload();
                m_FrameUBO.BindBase();
            }

            // ---- object UBO (binding 1) ----
            glm::mat4 modelM = glm::mat4(1.0f);
            modelM = glm::translate(modelM, m_ModelTranslate);
            modelM = glm::scale(modelM, glm::vec3(m_ModelScale));

            if (m_ObjectUBO.GetRendererID() != 0)
            {
                if (m_ObjectUBO.Has("u_Model"))
                    m_ObjectUBO.SetMat4("u_Model", &modelM[0][0], false);
                else if (m_ObjectUBO.Has("ObjectData.u_Model"))
                    m_ObjectUBO.SetMat4("ObjectData.u_Model", &modelM[0][0], false);

                m_ObjectUBO.Upload();
                m_ObjectUBO.BindBase();
            }

            // ---- draw all meshes ----
            m_DebugMat.Bind();

            const auto& meshes = m_Model->GetMeshes();
            for (const auto& mesh : meshes)
                mesh.Draw();
        }

        void OnImGuiRender() override
        {
            ImGui::Begin("Sponza Test");

            ImGui::Text("Path: %s", m_SponzaPath.string().c_str());
            ImGui::Text("Loaded: %s", m_Model ? "YES" : "NO");

            if (!m_Model)
                ImGui::TextColored(ImVec4(1,0.4f,0.4f,1), "Sponza file not found. Fix m_SponzaPath.");

            uint32_t meshCount = m_Model ? (uint32_t)m_Model->GetMeshes().size() : 0;
            ImGui::Text("Meshes: %u", meshCount);

            ImGui::Separator();
            ImGui::Checkbox("Pause Orbit", &m_PauseOrbit);
            ImGui::SliderFloat("Orbit Speed", &m_OrbitSpeed, 0.0f, 3.0f);
            ImGui::SliderFloat("Orbit Radius", &m_OrbitRadius, 2.0f, 150.0f);
            ImGui::SliderFloat("Orbit Height", &m_OrbitHeight, -10.0f, 50.0f);

            ImGui::Separator();
            ImGui::SliderFloat("Model Scale", &m_ModelScale, 0.001f, 5.0f);
            ImGui::DragFloat3("Model Translate", &m_ModelTranslate.x, 0.1f);
            ImGui::DragFloat3("Target", &m_SponzaTarget.x, 0.1f);

            ImGui::End();
        }

    private:
        float m_Time = 0.0f;

        Core::Renderer::Camera m_Camera;

        Core::Renderer::Material m_DebugMat;
        Core::Renderer::UniformBuffer m_FrameUBO;
        Core::Renderer::UniformBuffer m_ObjectUBO;

        std::shared_ptr<Core::Renderer::Model> m_Model;
        std::filesystem::path m_SponzaPath;

        glm::vec2 m_LastFB{0,0};

        // orbit controls
        bool  m_PauseOrbit = false;
        float m_OrbitAngle = 0.0f;
        float m_OrbitSpeed = 0.6f;
        float m_OrbitRadius = 25.0f;
        float m_OrbitHeight = 8.0f;
        glm::vec3 m_SponzaTarget{ 0.0f, 5.0f, 0.0f };

        // model transform
        float m_ModelScale = 0.01f;              // Sponza often needs 0.01 (depends on asset)
        glm::vec3 m_ModelTranslate{ 0.0f, 0.0f, 0.0f };
    };
}
