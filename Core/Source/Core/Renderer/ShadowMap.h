#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <vector>

namespace Core::Renderer
{
    // Shadow map for a single light
    class ShadowMap
    {
    public:
        ShadowMap() = default;
        ~ShadowMap();

        ShadowMap(const ShadowMap&) = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        ShadowMap(ShadowMap&& other) noexcept;
        ShadowMap& operator=(ShadowMap&& other) noexcept;

        // Create shadow map with given resolution
        void Create(uint32_t width, uint32_t height, bool isCubemap = false);
        void Destroy();

        // Bind for rendering (writing to shadow map)
        void BindForWriting(uint32_t cubemapFace = 0) const;
        
        // Bind for reading (sampling in shader)
        void BindForReading(uint32_t textureUnit) const;

        // Unbind framebuffer (return to default)
        static void UnbindFramebuffer();

        GLuint GetDepthTexture() const { return m_DepthTexture; }
        GLuint GetFramebuffer() const { return m_FBO; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        bool IsCubemap() const { return m_IsCubemap; }

    private:
        GLuint m_FBO = 0;
        GLuint m_DepthTexture = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        bool m_IsCubemap = false;
    };

    // Shadow map manager for multiple lights
    class ShadowMapManager
    {
    public:
        ShadowMapManager() = default;

        // Initialize with a maximum number of shadow maps and resolution
        void Init(uint32_t maxShadowMaps, uint32_t resolution = 1024);
        void Shutdown();

        // Allocate a shadow map for a light (returns index or -1 if full)
        int32_t AllocateShadowMap(bool isCubemap = false);
        
        // Free a shadow map by index
        void FreeShadowMap(uint32_t index);

        ShadowMap* GetShadowMap(uint32_t index);
        const ShadowMap* GetShadowMap(uint32_t index) const;

        uint32_t GetShadowMapCount() const;
        uint32_t GetResolution() const { return m_Resolution; }

    private:
        std::vector<ShadowMap> m_ShadowMaps;
        std::vector<bool> m_UsedSlots; // Track which slots are in use
        uint32_t m_Resolution = 1024;
        uint32_t m_MaxShadowMaps = 8;
    };
}
