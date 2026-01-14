#pragma once
#include <cstdint>
#include <vector>
#include <initializer_list>

namespace Core::Renderer
{
    enum class FramebufferTextureFormat : uint8_t
    {
        None = 0,

        // Color
        RGBA8,
        RGBA16F,
        RG16F,
        R32I,

        // Depth/Stencil
        Depth24Stencil8,
        Depth32F
    };

    struct FramebufferTextureSpec
    {
        FramebufferTextureFormat Format = FramebufferTextureFormat::None;

        // Defaults chosen for GBuffer (nearest + clamp).
        bool LinearFiltering = false;
        bool ClampToEdge = true;
    };

    struct FramebufferAttachmentSpec
    {
        std::vector<FramebufferTextureSpec> Attachments;
        FramebufferAttachmentSpec() = default;
        FramebufferAttachmentSpec(std::initializer_list<FramebufferTextureSpec> list)
            : Attachments(list) {
        }
    };

    struct FramebufferSpec
    {
        uint32_t Width = 0, Height = 0;
        uint32_t Samples = 1; // 1 = no MSAA
        bool SwapchainTarget = false;

        FramebufferAttachmentSpec Attachments;
    };

    class Framebuffer
    {
    public:
        Framebuffer() = default;
        explicit Framebuffer(const FramebufferSpec& spec);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        void Resize(uint32_t width, uint32_t height);
        void Invalidate();

        void Bind() const;
        static void Unbind();

        const FramebufferSpec& GetSpec() const { return m_Spec; }
        uint32_t GetRendererID() const { return m_FBO; }

        uint32_t GetColorAttachmentCount() const { return (uint32_t)m_ColorAttachments.size(); }
        uint32_t GetColorAttachmentID(uint32_t index) const;
        uint32_t GetDepthAttachmentID() const { return m_DepthAttachment; }

        // Helpers
        void BindColorTexture(uint32_t attachmentIndex, uint32_t slot) const;
        void BindDepthTexture(uint32_t slot) const;

        // Picking (only meaningful if attachment is R32I)
        int ReadPixel(uint32_t attachmentIndex, int x, int y) const;

        // Clear a color attachment (works for RGBA/float). For R32I, use ClearColorAttachmentInt.
        void ClearColorAttachment(uint32_t attachmentIndex, float r, float g, float b, float a) const;
        void ClearColorAttachmentFloat(uint32_t attachmentIndex, float value) const;
        void ClearColorAttachmentInt(uint32_t attachmentIndex, int value) const;

    private:
        FramebufferSpec m_Spec{};
        uint32_t m_FBO = 0;

        std::vector<FramebufferTextureSpec> m_ColorAttachmentSpecs;
        FramebufferTextureSpec m_DepthAttachmentSpec{};

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0;
    };
}
