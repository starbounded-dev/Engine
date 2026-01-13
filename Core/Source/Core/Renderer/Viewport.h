#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

namespace Core::Renderer
{
    class Camera;
    class Material;

    // Viewport framebuffer for rendering 3D content
    class Viewport
    {
    public:
        Viewport(uint32_t width = 1280, uint32_t height = 720);
        ~Viewport();

        // Resize viewport
        void Resize(uint32_t width, uint32_t height);
        
        // Bind for rendering
        void Bind() const;
        void Unbind() const;
        
        // Clear viewport
        void Clear(const glm::vec4& color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)) const;
        
        // Get framebuffer texture for ImGui display
        GLuint GetColorAttachment() const { return m_ColorAttachment; }
        GLuint GetDepthAttachment() const { return m_DepthAttachment; }
        
        // Get dimensions
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        float GetAspectRatio() const { return (float)m_Width / (float)m_Height; }
        
        // Render a preview sphere/cube with material
        void RenderPreviewSphere(std::shared_ptr<Material> material, float rotation = 0.0f);
        void RenderPreviewCube(std::shared_ptr<Material> material, float rotation = 0.0f);
        
        // Get/Set camera for preview
        void SetCamera(std::shared_ptr<Camera> camera) { m_Camera = camera; }
        std::shared_ptr<Camera> GetCamera() const { return m_Camera; }

    private:
        void CreateFramebuffer();
        void DeleteFramebuffer();
        void CreatePreviewMeshes();
        void DeletePreviewMeshes();

    private:
        uint32_t m_Width, m_Height;
        GLuint m_FBO = 0;
        GLuint m_ColorAttachment = 0;
        GLuint m_DepthAttachment = 0;
        
        // Preview meshes (simple sphere and cube)
        GLuint m_SphereVAO = 0, m_SphereVBO = 0, m_SphereEBO = 0;
        GLuint m_CubeVAO = 0, m_CubeVBO = 0, m_CubeEBO = 0;
        uint32_t m_SphereIndexCount = 0;
        uint32_t m_CubeIndexCount = 0;
        
        // Camera for preview rendering
        std::shared_ptr<Camera> m_Camera;
    };
}
