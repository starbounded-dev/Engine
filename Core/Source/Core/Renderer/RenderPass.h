#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Core::Renderer
{
    class RenderPass;

    // Render target attachment description
    struct AttachmentDesc
    {
        GLenum Format = GL_RGBA8;
        GLenum Type = GL_UNSIGNED_BYTE;
        bool IsDepth = false;
        bool IsStencil = false;
        GLint MinFilter = GL_LINEAR;
        GLint MagFilter = GL_LINEAR;
        GLint WrapS = GL_CLAMP_TO_EDGE;
        GLint WrapT = GL_CLAMP_TO_EDGE;
    };

    // Render target with multiple attachments
    class RenderTarget
    {
    public:
        RenderTarget() = default;
        ~RenderTarget();

        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        RenderTarget(RenderTarget&& other) noexcept;
        RenderTarget& operator=(RenderTarget&& other) noexcept;

        // Create render target with specified attachments
        void Create(uint32_t width, uint32_t height,
            const std::vector<AttachmentDesc>& colorAttachments,
            bool includeDepth = true,
            bool includeStencil = false);

        void Destroy();

        void Bind() const;
        static void Unbind();

        // Bind specific color attachment as texture for reading
        void BindColorAttachment(uint32_t index, uint32_t textureUnit) const;
        void BindDepthAttachment(uint32_t textureUnit) const;

        // Resize render target
        void Resize(uint32_t width, uint32_t height);

        GLuint GetFramebuffer() const { return m_FBO; }
        GLuint GetColorTexture(uint32_t index) const;
        GLuint GetDepthTexture() const { return m_DepthTexture; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetColorAttachmentCount() const { return static_cast<uint32_t>(m_ColorTextures.size()); }

    private:
        GLuint m_FBO = 0;
        std::vector<GLuint> m_ColorTextures;
        GLuint m_DepthTexture = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        std::vector<AttachmentDesc> m_ColorAttachmentDescs;
        bool m_HasDepth = false;
        bool m_HasStencil = false;
    };

    // Clear operation for render pass
    struct ClearOperation
    {
        bool ClearColor = true;
        bool ClearDepth = true;
        bool ClearStencil = false;
        glm::vec4 ClearColorValue = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float ClearDepthValue = 1.0f;
        int32_t ClearStencilValue = 0;
    };

    // Render pass execution callback
    using RenderPassExecuteCallback = std::function<void(RenderPass*)>;

    // Render pass for organizing rendering operations
    class RenderPass
    {
    public:
        RenderPass() = default;
        explicit RenderPass(const std::string& name);

        void SetName(const std::string& name) { m_Name = name; }
        const std::string& GetName() const { return m_Name; }

        // Set render target (nullptr for default framebuffer)
        void SetRenderTarget(RenderTarget* target) { m_RenderTarget = target; }
        RenderTarget* GetRenderTarget() const { return m_RenderTarget; }

        // Clear operations
        void SetClearOperation(const ClearOperation& clearOp) { m_ClearOperation = clearOp; }
        const ClearOperation& GetClearOperation() const { return m_ClearOperation; }

        // Viewport
        void SetViewport(int x, int y, int width, int height);
        void GetViewport(int& x, int& y, int& width, int& height) const;

        // Execute callback
        void SetExecuteCallback(RenderPassExecuteCallback callback) { m_ExecuteCallback = callback; }

        // Execute the render pass
        void Execute();

        // Enable/disable render pass
        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

    private:
        void PerformClear();
        void SetupViewport();
        void BindRenderTarget();

    private:
        std::string m_Name;
        RenderTarget* m_RenderTarget = nullptr;
        ClearOperation m_ClearOperation;
        RenderPassExecuteCallback m_ExecuteCallback;
        
        int m_ViewportX = 0;
        int m_ViewportY = 0;
        int m_ViewportWidth = 0;
        int m_ViewportHeight = 0;
        bool m_UseCustomViewport = false;
        bool m_Enabled = true;
    };

    // Render pipeline manages multiple render passes
    class RenderPipeline
    {
    public:
        RenderPipeline() = default;

        // Add render pass to the pipeline
        void AddPass(std::shared_ptr<RenderPass> pass);
        void InsertPass(size_t index, std::shared_ptr<RenderPass> pass);
        void RemovePass(size_t index);
        void RemovePass(const std::string& name);

        // Get render pass
        std::shared_ptr<RenderPass> GetPass(size_t index) const;
        std::shared_ptr<RenderPass> GetPass(const std::string& name) const;

        size_t GetPassCount() const { return m_Passes.size(); }

        // Execute all enabled render passes in order
        void Execute();

        // Clear all passes
        void Clear();

    private:
        std::vector<std::shared_ptr<RenderPass>> m_Passes;
    };
}
