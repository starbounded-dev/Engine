#pragma once
#include <glad/glad.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>

namespace Core::Renderer
{
    // Texture types
    enum class TextureType : uint8_t
    {
        Texture2D = 0,
        Texture3D,
        TextureCubemap,
        Texture2DArray
    };

    // Texture format descriptors
    struct TextureFormat
    {
        GLenum InternalFormat = GL_RGBA8;
        GLenum Format = GL_RGBA;
        GLenum Type = GL_UNSIGNED_BYTE;
    };

    // Texture creation parameters
    struct TextureParams
    {
        GLint MinFilter = GL_LINEAR_MIPMAP_LINEAR;
        GLint MagFilter = GL_LINEAR;
        GLint WrapS = GL_REPEAT;
        GLint WrapT = GL_REPEAT;
        GLint WrapR = GL_REPEAT;
        bool GenerateMipmaps = true;
        int MaxAnisotropy = 16;
    };

    // Advanced texture class supporting 2D, 3D, Cubemap, and Array textures
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        // Create 2D texture
        void Create2D(uint32_t width, uint32_t height, const TextureFormat& format,
            const void* data = nullptr, const TextureParams& params = TextureParams());

        // Create 3D texture
        void Create3D(uint32_t width, uint32_t height, uint32_t depth,
            const TextureFormat& format, const void* data = nullptr,
            const TextureParams& params = TextureParams());

        // Create cubemap from 6 faces (order: +X, -X, +Y, -Y, +Z, -Z)
        void CreateCubemap(uint32_t size, const TextureFormat& format,
            const std::vector<const void*>& faceData = {},
            const TextureParams& params = TextureParams());

        // Create 2D array texture
        void Create2DArray(uint32_t width, uint32_t height, uint32_t layers,
            const TextureFormat& format, const void* data = nullptr,
            const TextureParams& params = TextureParams());

        // Load cubemap from 6 image files
        static Texture LoadCubemapFromFiles(const std::vector<std::filesystem::path>& faces,
            const TextureParams& params = TextureParams());

        // Load 2D texture from file
        static Texture LoadFromFile(const std::filesystem::path& path,
            const TextureParams& params = TextureParams());

        void Destroy();

        void Bind(uint32_t slot = 0) const;
        static void Unbind(TextureType type, uint32_t slot = 0);

        GLuint GetHandle() const { return m_Handle; }
        TextureType GetType() const { return m_Type; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetDepth() const { return m_Depth; }

    private:
        void ApplyParameters(GLenum target, const TextureParams& params);
        void GenerateMipmaps(GLenum target);

    private:
        GLuint m_Handle = 0;
        TextureType m_Type = TextureType::Texture2D;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_Depth = 0; // For 3D textures or array layer count
        TextureFormat m_Format;
    };

    // Cubemap utility for environment mapping
    class EnvironmentMap
    {
    public:
        EnvironmentMap() = default;

        // Create from equirectangular HDR image
        void CreateFromEquirectangular(const std::filesystem::path& hdrPath, uint32_t cubemapSize = 1024);

        // Create from 6 separate images
        void CreateFromFaces(const std::vector<std::filesystem::path>& faces);

        // Generate irradiance map for diffuse lighting
        void GenerateIrradianceMap(uint32_t size = 32);

        // Generate prefiltered environment map for specular lighting
        void GeneratePrefilteredMap(uint32_t size = 128, uint32_t mipLevels = 5);

        // Generate BRDF lookup texture
        void GenerateBRDFLUT(uint32_t size = 512);

        const Texture& GetEnvironmentMap() const { return m_EnvironmentMap; }
        const Texture& GetIrradianceMap() const { return m_IrradianceMap; }
        const Texture& GetPrefilteredMap() const { return m_PrefilteredMap; }
        GLuint GetBRDFLUT() const { return m_BRDFLUT; }

    private:
        Texture m_EnvironmentMap;
        Texture m_IrradianceMap;
        Texture m_PrefilteredMap;
        GLuint m_BRDFLUT = 0;
    };

    // Texture atlas for batching
    class TextureAtlas
    {
    public:
        struct SubTexture
        {
            float u0, v0, u1, v1; // UV coordinates
            uint32_t Width, Height;
        };

        TextureAtlas() = default;

        // Create atlas from individual images
        void Create(const std::vector<std::filesystem::path>& textures, uint32_t atlasSize = 2048);

        // Get subtexture UV coordinates
        const SubTexture* GetSubTexture(size_t index) const;
        size_t GetSubTextureCount() const { return m_SubTextures.size(); }

        const Texture& GetAtlasTexture() const { return m_AtlasTexture; }

    private:
        Texture m_AtlasTexture;
        std::vector<SubTexture> m_SubTextures;
    };
}
