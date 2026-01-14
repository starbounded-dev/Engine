#include "Framebuffer.h"

#include <glad/glad.h>
#include <cassert>
#include <vector>

namespace Core::Renderer
{
    static bool IsDepthFormat(FramebufferTextureFormat fmt)
    {
        return fmt == FramebufferTextureFormat::Depth24Stencil8 || fmt == FramebufferTextureFormat::Depth32F;
    }

    static GLenum ToGLInternalFormat(FramebufferTextureFormat fmt)
    {
        switch (fmt)
        {
        case FramebufferTextureFormat::RGBA8:           return GL_RGBA8;
        case FramebufferTextureFormat::RGBA16F:         return GL_RGBA16F;
        case FramebufferTextureFormat::RG16F:           return GL_RG16F;
        case FramebufferTextureFormat::R32I:            return GL_R32I;
        case FramebufferTextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        case FramebufferTextureFormat::Depth32F:        return GL_DEPTH_COMPONENT32F;
        default:                                        return 0;
        }
    }

    static GLenum ToGLFormat(FramebufferTextureFormat fmt)
    {
        switch (fmt)
        {
        case FramebufferTextureFormat::RGBA8:   return GL_RGBA;
        case FramebufferTextureFormat::RGBA16F: return GL_RGBA;
        case FramebufferTextureFormat::RG16F:   return GL_RG;
        case FramebufferTextureFormat::R32I:    return GL_RED_INTEGER;

        case FramebufferTextureFormat::Depth24Stencil8: return GL_DEPTH_STENCIL;
        case FramebufferTextureFormat::Depth32F:        return GL_DEPTH_COMPONENT;
        default: return 0;
        }
    }

    static GLenum ToGLType(FramebufferTextureFormat fmt)
    {
        switch (fmt)
        {
        case FramebufferTextureFormat::RGBA8:   return GL_UNSIGNED_BYTE;
        case FramebufferTextureFormat::RGBA16F: return GL_FLOAT;
        case FramebufferTextureFormat::RG16F:   return GL_FLOAT;
        case FramebufferTextureFormat::R32I:    return GL_INT;

        case FramebufferTextureFormat::Depth24Stencil8: return GL_UNSIGNED_INT_24_8;
        case FramebufferTextureFormat::Depth32F:        return GL_FLOAT;
        default: return 0;
        }
    }

    static void SetupTextureParams(GLuint tex, const FramebufferTextureSpec& spec)
    {
        const GLint filter = spec.LinearFiltering ? GL_LINEAR : GL_NEAREST;
        glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, filter);
        glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, filter);

        const GLint wrap = spec.ClampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        glTextureParameteri(tex, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(tex, GL_TEXTURE_WRAP_T, wrap);
    }

    static void CreateColorAttachment(GLuint& outTex, const FramebufferSpec& fbSpec, const FramebufferTextureSpec& texSpec)
    {
        const bool msaa = fbSpec.Samples > 1;
        const GLenum internal = ToGLInternalFormat(texSpec.Format);
        assert(internal);

        if (!msaa)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &outTex);
            glTextureStorage2D(outTex, 1, internal, fbSpec.Width, fbSpec.Height); // immutable storage :contentReference[oaicite:0]{index=0}
            SetupTextureParams(outTex, texSpec);
        }
        else
        {
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &outTex);
            glTextureStorage2DMultisample(outTex, fbSpec.Samples, internal, fbSpec.Width, fbSpec.Height, GL_TRUE); // :contentReference[oaicite:1]{index=1}
        }
    }

    static void CreateDepthAttachment(GLuint& outTex, const FramebufferSpec& fbSpec, const FramebufferTextureSpec& texSpec)
    {
        const bool msaa = fbSpec.Samples > 1;
        const GLenum internal = ToGLInternalFormat(texSpec.Format);
        assert(internal);

        if (!msaa)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &outTex);
            glTextureStorage2D(outTex, 1, internal, fbSpec.Width, fbSpec.Height); // :contentReference[oaicite:2]{index=2}

            FramebufferTextureSpec depthParams = texSpec;
            depthParams.ClampToEdge = true;
            depthParams.LinearFiltering = false;
            SetupTextureParams(outTex, depthParams);

            // For normal depth sampling (not shadow-compare)
            glTextureParameteri(outTex, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        }
        else
        {
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &outTex);
            glTextureStorage2DMultisample(outTex, fbSpec.Samples, internal, fbSpec.Width, fbSpec.Height, GL_TRUE); // :contentReference[oaicite:3]{index=3}
        }
    }

    Framebuffer::Framebuffer(const FramebufferSpec& spec)
        : m_Spec(spec)
    {
        for (const auto& a : m_Spec.Attachments.Attachments)
        {
            if (IsDepthFormat(a.Format))
                m_DepthAttachmentSpec = a;
            else
                m_ColorAttachmentSpecs.push_back(a);
        }

        Invalidate();
    }

    Framebuffer::~Framebuffer()
    {
        if (m_FBO) glDeleteFramebuffers(1, &m_FBO);
        if (!m_ColorAttachments.empty())
            glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
        if (m_DepthAttachment)
            glDeleteTextures(1, &m_DepthAttachment);
    }

    void Framebuffer::Invalidate()
    {
        if (m_Spec.SwapchainTarget)
            return;

        // cleanup
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        if (!m_ColorAttachments.empty())
        {
            glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
            m_ColorAttachments.clear();
        }
        if (m_DepthAttachment)
        {
            glDeleteTextures(1, &m_DepthAttachment);
            m_DepthAttachment = 0;
        }

        glCreateFramebuffers(1, &m_FBO); // :contentReference[oaicite:4]{index=4}

        // color
        if (!m_ColorAttachmentSpecs.empty())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());

            for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++)
            {
                CreateColorAttachment(m_ColorAttachments[i], m_Spec, m_ColorAttachmentSpecs[i]);
                glNamedFramebufferTexture(m_FBO, GL_COLOR_ATTACHMENT0 + (GLenum)i, m_ColorAttachments[i], 0);
            }

            std::vector<GLenum> drawBuffers;
            drawBuffers.reserve(m_ColorAttachments.size());
            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + (GLenum)i);

            glNamedFramebufferDrawBuffers(m_FBO, (GLsizei)drawBuffers.size(), drawBuffers.data());
        }
        else
        {
            glNamedFramebufferDrawBuffer(m_FBO, GL_NONE);
            glNamedFramebufferReadBuffer(m_FBO, GL_NONE);
        }

        // depth
        if (m_DepthAttachmentSpec.Format != FramebufferTextureFormat::None)
        {
            CreateDepthAttachment(m_DepthAttachment, m_Spec, m_DepthAttachmentSpec);

            if (m_DepthAttachmentSpec.Format == FramebufferTextureFormat::Depth24Stencil8)
                glNamedFramebufferTexture(m_FBO, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAttachment, 0);
            else
                glNamedFramebufferTexture(m_FBO, GL_DEPTH_ATTACHMENT, m_DepthAttachment, 0);
        }

        const GLenum status = glCheckNamedFramebufferStatus(m_FBO, GL_FRAMEBUFFER); // :contentReference[oaicite:5]{index=5}
        assert(status == GL_FRAMEBUFFER_COMPLETE && "Framebuffer incomplete!");
    }

    void Framebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;
        if (width == m_Spec.Width && height == m_Spec.Height) return;

        m_Spec.Width = width;
        m_Spec.Height = height;
        Invalidate();
    }

    void Framebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, (GLsizei)m_Spec.Width, (GLsizei)m_Spec.Height);
    }

    void Framebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    uint32_t Framebuffer::GetColorAttachmentID(uint32_t index) const
    {
        assert(index < m_ColorAttachments.size());
        return m_ColorAttachments[index];
    }

    void Framebuffer::BindColorTexture(uint32_t attachmentIndex, uint32_t slot) const
    {
        assert(attachmentIndex < m_ColorAttachments.size());
        glBindTextureUnit(slot, m_ColorAttachments[attachmentIndex]); // :contentReference[oaicite:6]{index=6}
    }

    void Framebuffer::BindDepthTexture(uint32_t slot) const
    {
        assert(m_DepthAttachment != 0);
        glBindTextureUnit(slot, m_DepthAttachment); // :contentReference[oaicite:7]{index=7}
    }

    int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) const
    {
        assert(attachmentIndex < m_ColorAttachments.size());
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

        int pixel = 0;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        return pixel;
    }

    void Framebuffer::ClearColorAttachment(uint32_t attachmentIndex, float r, float g, float b, float a) const
    {
        assert(attachmentIndex < m_ColorAttachments.size());
        const float v[4] = { r, g, b, a };

        auto fmt = m_ColorAttachmentSpecs[attachmentIndex].Format;
        glClearTexImage(m_ColorAttachments[attachmentIndex], 0, ToGLFormat(fmt), ToGLType(fmt), v);
    }

    void Framebuffer::ClearColorAttachmentFloat(uint32_t attachmentIndex, float value) const
    {
        ClearColorAttachment(attachmentIndex, value, value, value, value);
    }

    void Framebuffer::ClearColorAttachmentInt(uint32_t attachmentIndex, int value) const
    {
        assert(attachmentIndex < m_ColorAttachments.size());
        auto fmt = m_ColorAttachmentSpecs[attachmentIndex].Format;
        glClearTexImage(m_ColorAttachments[attachmentIndex], 0, ToGLFormat(fmt), ToGLType(fmt), &value);
    }
}
