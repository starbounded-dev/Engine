#include "RenderPass.h"
#include <algorithm>

namespace Core::Renderer
{
    // RenderTarget implementation
    RenderTarget::~RenderTarget()
    {
        Destroy();
    }

    RenderTarget::RenderTarget(RenderTarget&& other) noexcept
        : m_FBO(other.m_FBO)
        , m_ColorTextures(std::move(other.m_ColorTextures))
        , m_DepthTexture(other.m_DepthTexture)
        , m_Width(other.m_Width)
        , m_Height(other.m_Height)
        , m_ColorAttachmentDescs(std::move(other.m_ColorAttachmentDescs))
        , m_HasDepth(other.m_HasDepth)
        , m_HasStencil(other.m_HasStencil)
    {
        other.m_FBO = 0;
        other.m_DepthTexture = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_HasDepth = false;
        other.m_HasStencil = false;
    }

    RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_FBO = other.m_FBO;
            m_ColorTextures = std::move(other.m_ColorTextures);
            m_DepthTexture = other.m_DepthTexture;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_ColorAttachmentDescs = std::move(other.m_ColorAttachmentDescs);
            m_HasDepth = other.m_HasDepth;
            m_HasStencil = other.m_HasStencil;

            other.m_FBO = 0;
            other.m_DepthTexture = 0;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_HasDepth = false;
            other.m_HasStencil = false;
        }
        return *this;
    }

    void RenderTarget::Create(uint32_t width, uint32_t height,
        const std::vector<AttachmentDesc>& colorAttachments,
        bool includeDepth, bool includeStencil)
    {
        Destroy();

        m_Width = width;
        m_Height = height;
        m_ColorAttachmentDescs = colorAttachments;
        m_HasDepth = includeDepth;
        m_HasStencil = includeStencil;

        // Create framebuffer
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // Create color attachments
        m_ColorTextures.resize(colorAttachments.size());
        std::vector<GLenum> drawBuffers;

        for (size_t i = 0; i < colorAttachments.size(); ++i)
        {
            const auto& desc = colorAttachments[i];
            
            glGenTextures(1, &m_ColorTextures[i]);
            glBindTexture(GL_TEXTURE_2D, m_ColorTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, desc.Format, width, height, 0,
                GL_RGBA, desc.Type, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.MinFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desc.MagFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, desc.WrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, desc.WrapT);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                GL_TEXTURE_2D, m_ColorTextures[i], 0);

            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i));
        }

        if (!drawBuffers.empty())
        {
            glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        }

        // Create depth/stencil attachment
        if (includeDepth || includeStencil)
        {
            glGenTextures(1, &m_DepthTexture);
            glBindTexture(GL_TEXTURE_2D, m_DepthTexture);

            GLenum format = includeDepth && includeStencil ? GL_DEPTH24_STENCIL8 :
                            includeDepth ? GL_DEPTH_COMPONENT24 : GL_STENCIL_INDEX8;
            GLenum attachment = includeDepth && includeStencil ? GL_DEPTH_STENCIL_ATTACHMENT :
                                includeDepth ? GL_DEPTH_ATTACHMENT : GL_STENCIL_ATTACHMENT;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                includeDepth ? GL_DEPTH_COMPONENT : GL_STENCIL_INDEX, GL_FLOAT, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, m_DepthTexture, 0);
        }

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Destroy();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderTarget::Destroy()
    {
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }

        if (!m_ColorTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(m_ColorTextures.size()), m_ColorTextures.data());
            m_ColorTextures.clear();
        }

        if (m_DepthTexture)
        {
            glDeleteTextures(1, &m_DepthTexture);
            m_DepthTexture = 0;
        }

        m_Width = 0;
        m_Height = 0;
        m_HasDepth = false;
        m_HasStencil = false;
    }

    void RenderTarget::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void RenderTarget::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderTarget::BindColorAttachment(uint32_t index, uint32_t textureUnit) const
    {
        if (index < m_ColorTextures.size())
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, m_ColorTextures[index]);
        }
    }

    void RenderTarget::BindDepthAttachment(uint32_t textureUnit) const
    {
        if (m_DepthTexture)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
        }
    }

    void RenderTarget::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width == width && m_Height == height)
            return;

        Create(width, height, m_ColorAttachmentDescs, m_HasDepth, m_HasStencil);
    }

    GLuint RenderTarget::GetColorTexture(uint32_t index) const
    {
        if (index < m_ColorTextures.size())
        {
            return m_ColorTextures[index];
        }
        return 0;
    }

    // RenderPass implementation
    RenderPass::RenderPass(const std::string& name)
        : m_Name(name)
    {
    }

    void RenderPass::SetViewport(int x, int y, int width, int height)
    {
        m_ViewportX = x;
        m_ViewportY = y;
        m_ViewportWidth = width;
        m_ViewportHeight = height;
        m_UseCustomViewport = true;
    }

    void RenderPass::GetViewport(int& x, int& y, int& width, int& height) const
    {
        x = m_ViewportX;
        y = m_ViewportY;
        width = m_ViewportWidth;
        height = m_ViewportHeight;
    }

    void RenderPass::Execute()
    {
        if (!m_Enabled)
            return;

        BindRenderTarget();
        SetupViewport();
        PerformClear();

        if (m_ExecuteCallback)
        {
            m_ExecuteCallback(this);
        }
    }

    void RenderPass::PerformClear()
    {
        GLbitfield clearBits = 0;

        if (m_ClearOperation.ClearColor)
        {
            glClearColor(m_ClearOperation.ClearColorValue.r,
                m_ClearOperation.ClearColorValue.g,
                m_ClearOperation.ClearColorValue.b,
                m_ClearOperation.ClearColorValue.a);
            clearBits |= GL_COLOR_BUFFER_BIT;
        }

        if (m_ClearOperation.ClearDepth)
        {
            glClearDepth(m_ClearOperation.ClearDepthValue);
            clearBits |= GL_DEPTH_BUFFER_BIT;
        }

        if (m_ClearOperation.ClearStencil)
        {
            glClearStencil(m_ClearOperation.ClearStencilValue);
            clearBits |= GL_STENCIL_BUFFER_BIT;
        }

        if (clearBits != 0)
        {
            glClear(clearBits);
        }
    }

    void RenderPass::SetupViewport()
    {
        if (m_UseCustomViewport)
        {
            glViewport(m_ViewportX, m_ViewportY, m_ViewportWidth, m_ViewportHeight);
        }
        else if (m_RenderTarget)
        {
            glViewport(0, 0, m_RenderTarget->GetWidth(), m_RenderTarget->GetHeight());
        }
    }

    void RenderPass::BindRenderTarget()
    {
        if (m_RenderTarget)
        {
            m_RenderTarget->Bind();
        }
        else
        {
            RenderTarget::Unbind();
        }
    }

    // RenderPipeline implementation
    void RenderPipeline::AddPass(std::shared_ptr<RenderPass> pass)
    {
        m_Passes.push_back(pass);
    }

    void RenderPipeline::InsertPass(size_t index, std::shared_ptr<RenderPass> pass)
    {
        if (index <= m_Passes.size())
        {
            m_Passes.insert(m_Passes.begin() + index, pass);
        }
    }

    void RenderPipeline::RemovePass(size_t index)
    {
        if (index < m_Passes.size())
        {
            m_Passes.erase(m_Passes.begin() + index);
        }
    }

    void RenderPipeline::RemovePass(const std::string& name)
    {
        auto it = std::find_if(m_Passes.begin(), m_Passes.end(),
            [&name](const std::shared_ptr<RenderPass>& pass) {
                return pass->GetName() == name;
            });

        if (it != m_Passes.end())
        {
            m_Passes.erase(it);
        }
    }

    std::shared_ptr<RenderPass> RenderPipeline::GetPass(size_t index) const
    {
        if (index < m_Passes.size())
        {
            return m_Passes[index];
        }
        return nullptr;
    }

    std::shared_ptr<RenderPass> RenderPipeline::GetPass(const std::string& name) const
    {
        auto it = std::find_if(m_Passes.begin(), m_Passes.end(),
            [&name](const std::shared_ptr<RenderPass>& pass) {
                return pass->GetName() == name;
            });

        if (it != m_Passes.end())
        {
            return *it;
        }
        return nullptr;
    }

    void RenderPipeline::Execute()
    {
        for (auto& pass : m_Passes)
        {
            if (pass && pass->IsEnabled())
            {
                pass->Execute();
            }
        }
    }

    void RenderPipeline::Clear()
    {
        m_Passes.clear();
    }
}
