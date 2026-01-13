#include "Texture.h"
#include <iostream>

// Include stb_image for loading
#include "stb_image.h"

namespace Core::Renderer
{
    // Texture implementation
    Texture::~Texture()
    {
        Destroy();
    }

    Texture::Texture(Texture&& other) noexcept
        : m_Handle(other.m_Handle)
        , m_Type(other.m_Type)
        , m_Width(other.m_Width)
        , m_Height(other.m_Height)
        , m_Depth(other.m_Depth)
        , m_Format(other.m_Format)
    {
        other.m_Handle = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Depth = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_Handle = other.m_Handle;
            m_Type = other.m_Type;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Depth = other.m_Depth;
            m_Format = other.m_Format;

            other.m_Handle = 0;
            other.m_Width = 0;
            other.m_Height = 0;
            other.m_Depth = 0;
        }
        return *this;
    }

    void Texture::Create2D(uint32_t width, uint32_t height, const TextureFormat& format,
        const void* data, const TextureParams& params)
    {
        Destroy();

        m_Type = TextureType::Texture2D;
        m_Width = width;
        m_Height = height;
        m_Format = format;

        glGenTextures(1, &m_Handle);
        glBindTexture(GL_TEXTURE_2D, m_Handle);

        glTexImage2D(GL_TEXTURE_2D, 0, format.InternalFormat, width, height, 0,
            format.Format, format.Type, data);

        ApplyParameters(GL_TEXTURE_2D, params);

        if (params.GenerateMipmaps)
        {
            GenerateMipmaps(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth,
        const TextureFormat& format, const void* data, const TextureParams& params)
    {
        Destroy();

        m_Type = TextureType::Texture3D;
        m_Width = width;
        m_Height = height;
        m_Depth = depth;
        m_Format = format;

        glGenTextures(1, &m_Handle);
        glBindTexture(GL_TEXTURE_3D, m_Handle);

        glTexImage3D(GL_TEXTURE_3D, 0, format.InternalFormat, width, height, depth, 0,
            format.Format, format.Type, data);

        ApplyParameters(GL_TEXTURE_3D, params);

        if (params.GenerateMipmaps)
        {
            GenerateMipmaps(GL_TEXTURE_3D);
        }

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    void Texture::CreateCubemap(uint32_t size, const TextureFormat& format,
        const std::vector<const void*>& faceData, const TextureParams& params)
    {
        Destroy();

        m_Type = TextureType::TextureCubemap;
        m_Width = size;
        m_Height = size;
        m_Format = format;

        glGenTextures(1, &m_Handle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_Handle);

        for (uint32_t i = 0; i < 6; ++i)
        {
            const void* data = (i < faceData.size()) ? faceData[i] : nullptr;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format.InternalFormat,
                size, size, 0, format.Format, format.Type, data);
        }

        ApplyParameters(GL_TEXTURE_CUBE_MAP, params);

        if (params.GenerateMipmaps)
        {
            GenerateMipmaps(GL_TEXTURE_CUBE_MAP);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void Texture::Create2DArray(uint32_t width, uint32_t height, uint32_t layers,
        const TextureFormat& format, const void* data, const TextureParams& params)
    {
        Destroy();

        m_Type = TextureType::Texture2DArray;
        m_Width = width;
        m_Height = height;
        m_Depth = layers;
        m_Format = format;

        glGenTextures(1, &m_Handle);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_Handle);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format.InternalFormat, width, height, layers, 0,
            format.Format, format.Type, data);

        ApplyParameters(GL_TEXTURE_2D_ARRAY, params);

        if (params.GenerateMipmaps)
        {
            GenerateMipmaps(GL_TEXTURE_2D_ARRAY);
        }

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    Texture Texture::LoadCubemapFromFiles(const std::vector<std::filesystem::path>& faces,
        const TextureParams& params)
    {
        Texture texture;

        if (faces.size() != 6)
        {
            std::cerr << "Cubemap requires exactly 6 face images\n";
            return texture;
        }

        std::vector<unsigned char*> faceDataRaw(6);
        int width = 0, height = 0, channels = 0;

        // Load all 6 faces
        for (size_t i = 0; i < 6; ++i)
        {
            int w, h, c;
            faceDataRaw[i] = stbi_load(faces[i].string().c_str(), &w, &h, &c, 0);

            if (!faceDataRaw[i])
            {
                std::cerr << "Failed to load cubemap face: " << faces[i] << "\n";
                // Clean up previously loaded faces
                for (size_t j = 0; j < i; ++j)
                {
                    stbi_image_free(faceDataRaw[j]);
                }
                return texture;
            }

            if (i == 0)
            {
                width = w;
                height = h;
                channels = c;
            }
            else if (w != width || h != height)
            {
                std::cerr << "All cubemap faces must have the same dimensions\n";
                for (size_t j = 0; j <= i; ++j)
                {
                    stbi_image_free(faceDataRaw[j]);
                }
                return texture;
            }
        }

        // Create texture format
        TextureFormat format;
        format.Format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
        format.InternalFormat = (channels == 4) ? GL_RGBA8 : (channels == 3) ? GL_RGB8 : GL_R8;
        format.Type = GL_UNSIGNED_BYTE;

        // Convert raw pointers to const void*
        std::vector<const void*> faceData(6);
        for (size_t i = 0; i < 6; ++i)
        {
            faceData[i] = faceDataRaw[i];
        }

        texture.CreateCubemap(width, format, faceData, params);

        // Free loaded data
        for (auto data : faceDataRaw)
        {
            stbi_image_free(data);
        }

        return texture;
    }

    Texture Texture::LoadFromFile(const std::filesystem::path& path, const TextureParams& params)
    {
        Texture texture;

        int width, height, channels;
        unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << path << "\n";
            return texture;
        }

        TextureFormat format;
        format.Format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;
        format.InternalFormat = (channels == 4) ? GL_RGBA8 : (channels == 3) ? GL_RGB8 : GL_R8;
        format.Type = GL_UNSIGNED_BYTE;

        texture.Create2D(width, height, format, data, params);

        stbi_image_free(data);

        return texture;
    }

    void Texture::Destroy()
    {
        if (m_Handle)
        {
            glDeleteTextures(1, &m_Handle);
            m_Handle = 0;
        }
        m_Width = 0;
        m_Height = 0;
        m_Depth = 0;
    }

    void Texture::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);

        GLenum target = GL_TEXTURE_2D;
        switch (m_Type)
        {
        case TextureType::Texture2D: target = GL_TEXTURE_2D; break;
        case TextureType::Texture3D: target = GL_TEXTURE_3D; break;
        case TextureType::TextureCubemap: target = GL_TEXTURE_CUBE_MAP; break;
        case TextureType::Texture2DArray: target = GL_TEXTURE_2D_ARRAY; break;
        }

        glBindTexture(target, m_Handle);
    }

    void Texture::Unbind(TextureType type, uint32_t slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);

        GLenum target = GL_TEXTURE_2D;
        switch (type)
        {
        case TextureType::Texture2D: target = GL_TEXTURE_2D; break;
        case TextureType::Texture3D: target = GL_TEXTURE_3D; break;
        case TextureType::TextureCubemap: target = GL_TEXTURE_CUBE_MAP; break;
        case TextureType::Texture2DArray: target = GL_TEXTURE_2D_ARRAY; break;
        }

        glBindTexture(target, 0);
    }

    void Texture::ApplyParameters(GLenum target, const TextureParams& params)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, params.MinFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, params.MagFilter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, params.WrapS);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, params.WrapT);

        if (target == GL_TEXTURE_3D || target == GL_TEXTURE_CUBE_MAP)
        {
            glTexParameteri(target, GL_TEXTURE_WRAP_R, params.WrapR);
        }

        // Anisotropic filtering if supported
        if (params.MaxAnisotropy > 1)
        {
            float maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
            float aniso = std::min(static_cast<float>(params.MaxAnisotropy), maxAniso);
            glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, aniso);
        }
    }

    void Texture::GenerateMipmaps(GLenum target)
    {
        glGenerateMipmap(target);
    }

    // EnvironmentMap implementation (placeholder for IBL)
    void EnvironmentMap::CreateFromEquirectangular(const std::filesystem::path& hdrPath, uint32_t cubemapSize)
    {
        // Placeholder: Load HDR image and convert to cubemap
        // This would require a shader to sample the equirectangular map onto cubemap faces
        std::cerr << "EnvironmentMap::CreateFromEquirectangular not fully implemented yet\n";
    }

    void EnvironmentMap::CreateFromFaces(const std::vector<std::filesystem::path>& faces)
    {
        m_EnvironmentMap = Texture::LoadCubemapFromFiles(faces);
    }

    void EnvironmentMap::GenerateIrradianceMap(uint32_t size)
    {
        // Placeholder: Convolve environment map for diffuse irradiance
        std::cerr << "EnvironmentMap::GenerateIrradianceMap not fully implemented yet\n";
    }

    void EnvironmentMap::GeneratePrefilteredMap(uint32_t size, uint32_t mipLevels)
    {
        // Placeholder: Generate prefiltered specular map with varying roughness
        std::cerr << "EnvironmentMap::GeneratePrefilteredMap not fully implemented yet\n";
    }

    void EnvironmentMap::GenerateBRDFLUT(uint32_t size)
    {
        // Placeholder: Generate BRDF integration lookup texture
        std::cerr << "EnvironmentMap::GenerateBRDFLUT not fully implemented yet\n";
    }

    // TextureAtlas implementation (basic)
    void TextureAtlas::Create(const std::vector<std::filesystem::path>& textures, uint32_t atlasSize)
    {
        // Placeholder: Pack textures into a single atlas
        // This would require a bin-packing algorithm
        std::cerr << "TextureAtlas::Create not fully implemented yet\n";
    }

    const TextureAtlas::SubTexture* TextureAtlas::GetSubTexture(size_t index) const
    {
        if (index < m_SubTextures.size())
        {
            return &m_SubTextures[index];
        }
        return nullptr;
    }
}
