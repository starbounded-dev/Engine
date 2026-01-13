#include "ShadowMap.h"
#include <algorithm>

namespace Core::Renderer
{
    ShadowMap::~ShadowMap()
    {
        Destroy();
    }

    ShadowMap::ShadowMap(ShadowMap&& other) noexcept
        : m_FBO(other.m_FBO)
        , m_DepthTexture(other.m_DepthTexture)
        , m_Width(other.m_Width)
        , m_Height(other.m_Height)
        , m_IsCubemap(other.m_IsCubemap)
    {
        other.m_FBO = 0;
        other.m_DepthTexture = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_IsCubemap = false;
    }

    ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_FBO = other.m_FBO;
            m_DepthTexture = other.m_DepthTexture;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_IsCubemap = other.m_IsCubemap;

            other.m_FBO = 0;
            other.m_DepthTexture = 0;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_IsCubemap = false;
        }
        return *this;
    }

    void ShadowMap::Create(uint32_t width, uint32_t height, bool isCubemap)
    {
        Destroy();

        m_Width = width;
        m_Height = height;
        m_IsCubemap = isCubemap;

        // Create depth texture
        GLenum target = isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        glGenTextures(1, &m_DepthTexture);
        glBindTexture(target, m_DepthTexture);

        if (isCubemap)
        {
            // Create cubemap faces
            for (uint32_t i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                    width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }

        // Texture parameters
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        if (isCubemap)
        {
            glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        }

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Create framebuffer
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        if (isCubemap)
        {
            // For cubemap, we'll attach faces during rendering
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
        }

        // No color attachment needed for shadow mapping
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            // Error handling
            Destroy();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ShadowMap::Destroy()
    {
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }

        if (m_DepthTexture)
        {
            glDeleteTextures(1, &m_DepthTexture);
            m_DepthTexture = 0;
        }

        m_Width = 0;
        m_Height = 0;
        m_IsCubemap = false;
    }

    void ShadowMap::BindForWriting(uint32_t cubemapFace) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, m_Width, m_Height);

        if (m_IsCubemap && cubemapFace < 6)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapFace, m_DepthTexture, 0);
        }

        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void ShadowMap::BindForReading(uint32_t textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        GLenum target = m_IsCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        glBindTexture(target, m_DepthTexture);
    }

    void ShadowMap::UnbindFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ShadowMapManager implementation
    void ShadowMapManager::Init(uint32_t maxShadowMaps, uint32_t resolution)
    {
        m_MaxShadowMaps = maxShadowMaps;
        m_Resolution = resolution;
        m_ShadowMaps.clear();
        m_ShadowMaps.reserve(maxShadowMaps);
    }

    void ShadowMapManager::Shutdown()
    {
        m_ShadowMaps.clear();
    }

    uint32_t ShadowMapManager::AllocateShadowMap(bool isCubemap)
    {
        if (m_ShadowMaps.size() >= m_MaxShadowMaps)
        {
            return static_cast<uint32_t>(-1); // Max capacity reached
        }

        ShadowMap shadowMap;
        shadowMap.Create(m_Resolution, m_Resolution, isCubemap);
        
        m_ShadowMaps.push_back(std::move(shadowMap));
        return static_cast<uint32_t>(m_ShadowMaps.size() - 1);
    }

    void ShadowMapManager::FreeShadowMap(uint32_t index)
    {
        if (index < m_ShadowMaps.size())
        {
            m_ShadowMaps.erase(m_ShadowMaps.begin() + index);
        }
    }

    ShadowMap* ShadowMapManager::GetShadowMap(uint32_t index)
    {
        if (index < m_ShadowMaps.size())
        {
            return &m_ShadowMaps[index];
        }
        return nullptr;
    }

    const ShadowMap* ShadowMapManager::GetShadowMap(uint32_t index) const
    {
        if (index < m_ShadowMaps.size())
        {
            return &m_ShadowMaps[index];
        }
        return nullptr;
    }
}
