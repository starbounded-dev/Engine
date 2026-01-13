#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <vector>
#include <cstdint>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Core/Renderer/UniformBuffer.h"

namespace Core::Renderer
{
    class Material;

    using MaterialValue = std::variant<
        float, int32_t, uint32_t,
        glm::vec2, glm::vec3, glm::vec4,
        glm::mat3, glm::mat4
    >;

    struct TextureBinding
    {
        std::string Uniform;     // sampler uniform name, e.g. "u_Albedo"
        uint32_t    Slot = 0;    // texture unit
        GLuint      TextureID = 0;
        GLenum      Target = GL_TEXTURE_2D;
    };

    // Shared GPU resources (shader program + reflection info)
    struct MaterialResources
    {
        GLuint Program = 0;

        std::string VertexPath;
        std::string FragmentPath;

        std::string MaterialBlockName = "MaterialData";
        UniformBufferLayout MaterialLayout;

        // Cache for non-UBO uniforms (samplers, misc)
        mutable std::unordered_map<std::string, GLint> UniformLocationCache;
    };

    class MaterialInstance;

    class Material : public std::enable_shared_from_this<Material>
    {
    	friend class MaterialInstance;

    public:
        Material() = default;
        Material(const std::string& vertexPath, const std::string& fragmentPath);
        ~Material();

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

        Material(Material&&) noexcept;
        Material& operator=(Material&&) noexcept;

        void Bind() const;

        // UBO-backed parameters (MaterialData). If not found in block, falls back to glUniform.
        void SetFloat(const std::string& name, float v);
        void SetInt(const std::string& name, int32_t v);
        void SetUInt(const std::string& name, uint32_t v);
        void SetVec2(const std::string& name, const glm::vec2& v);
        void SetVec3(const std::string& name, const glm::vec3& v);
        void SetVec4(const std::string& name, const glm::vec4& v);
        void SetMat3(const std::string& name, const glm::mat3& v);
        void SetMat4(const std::string& name, const glm::mat4& v);

        // Texture binding (samplers)
        // If slot not used yet, pass whatever slot you want (0..)
        void SetTexture(const std::string& samplerUniform, uint32_t slot, GLuint textureID, GLenum target = GL_TEXTURE_2D);

        // Instancing: share shader + defaults, but instance can override params/textures
        std::shared_ptr<MaterialInstance> CreateInstance() const;

        // ImGui material editor (basic)
        void OnImGuiRender(const char* label = nullptr);

        // Access
        GLuint GetProgram() const;
        const std::unordered_map<std::string, MaterialValue>& GetValues() const { return m_Values; }
        const std::vector<TextureBinding>& GetTextures() const { return m_Textures; }

    public:
        void Rebuild(); // compile + link + reflect
        void Destroy();

        // UBO helpers (try name, then "MaterialData.name")
        bool HasMaterialUBO(const std::string& name) const;
        std::string ResolveMaterialUBOName(const std::string& name) const;

        // Non-UBO uniform helpers
        GLint GetUniformLocationCached(const std::string& name) const;
        void SetUniformFallback(const std::string& name, const MaterialValue& v) const;

        // Apply texture sampler uniforms + bind textures
        void BindTextures() const;

    private:
        std::shared_ptr<MaterialResources> m_Res;

        // Per-material buffer (binding = 2)
        UniformBuffer m_MaterialUBO;

        // Defaults (editor reads these)
        std::unordered_map<std::string, MaterialValue> m_Values;
        std::vector<TextureBinding> m_Textures;
    };

    class MaterialInstance
    {
    public:
        explicit MaterialInstance(std::shared_ptr<const Material> base);

        void Bind() const;

        void SetFloat(const std::string& name, float v);
        void SetInt(const std::string& name, int32_t v);
        void SetUInt(const std::string& name, uint32_t v);
        void SetVec2(const std::string& name, const glm::vec2& v);
        void SetVec3(const std::string& name, const glm::vec3& v);
        void SetVec4(const std::string& name, const glm::vec4& v);
        void SetMat3(const std::string& name, const glm::mat3& v);
        void SetMat4(const std::string& name, const glm::mat4& v);

        void SetTexture(const std::string& samplerUniform, uint32_t slot, GLuint textureID, GLenum target = GL_TEXTURE_2D);

        void OnImGuiRender(const char* label = nullptr);

    private:
        bool HasMaterialUBO(const std::string& name) const;
        std::string ResolveMaterialUBOName(const std::string& name) const;

        void BindTextures() const;

    private:
        std::shared_ptr<const Material> m_Base;

        // Instance overrides (only what you set)
        std::unordered_map<std::string, MaterialValue> m_Overrides;
        std::vector<TextureBinding> m_TextureOverrides;

        // Each instance has its own material UBO bound at binding=2 before draw
        UniformBuffer m_InstanceUBO;
    };
}
