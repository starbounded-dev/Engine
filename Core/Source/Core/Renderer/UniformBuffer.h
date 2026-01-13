#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include <glad/glad.h>

namespace Core::Renderer
{
    // Recommended binding points (keep consistent with your GLSL: layout(binding = X))
    namespace UBOBinding
    {
        inline constexpr uint32_t PerFrame = 0;
        inline constexpr uint32_t PerObject = 1;
        inline constexpr uint32_t PerMaterial = 2;
    }

    struct UniformBufferElement
    {
        std::string Name;

        uint32_t Offset = 0;
        uint32_t Size = 0;

        // Useful for arrays/matrices (optional)
        uint32_t ArrayStride = 0;
        uint32_t MatrixStride = 0;

        GLenum Type = 0;
    };

    class UniformBufferLayout
    {
    public:
        UniformBufferLayout() = default;

        uint32_t GetSize() const { return m_Size; }
        bool Has(const std::string& name) const { return m_Elements.find(name) != m_Elements.end(); }

        const UniformBufferElement* Find(const std::string& name) const;

        // Reflect a GLSL uniform block (std140 or std430) from a linked program
        // blockName example: "FrameData" / "ObjectData" / "MaterialData"
        static UniformBufferLayout Reflect(GLuint program, const std::string& blockName);

    private:
        uint32_t m_Size = 0;
        std::unordered_map<std::string, UniformBufferElement> m_Elements;
    };

    class UniformBuffer
    {
    public:
        UniformBuffer() = default;
        UniformBuffer(uint32_t size, uint32_t bindingPoint, bool dynamic = true);
        UniformBuffer(const UniformBufferLayout& layout, uint32_t bindingPoint, bool dynamic = true);
        ~UniformBuffer();

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        UniformBuffer(UniformBuffer&& other) noexcept;
        UniformBuffer& operator=(UniformBuffer&& other) noexcept;

        void BindBase() const; // glBindBufferBase(GL_UNIFORM_BUFFER, binding, id)

        uint32_t GetSize() const { return m_Size; }
        uint32_t GetBindingPoint() const { return m_BindingPoint; }
        GLuint   GetRendererID() const { return m_RendererID; }

        const UniformBufferLayout& GetLayout() const { return m_Layout; }
        bool Has(const std::string& name) const { return m_Layout.Has(name); }

        // Raw update (offset in bytes)
        void SetData(const void* data, uint32_t size, uint32_t offset = 0);

        // Structured update by uniform name (requires layout)
        void SetFloat(const std::string& name, float v, bool uploadNow = true);
        void SetInt(const std::string& name, int v, bool uploadNow = true);
        void SetUInt(const std::string& name, uint32_t v, bool uploadNow = true);

        void SetVec2(const std::string& name, const float v[2], bool uploadNow = true);
        void SetVec3(const std::string& name, const float v[3], bool uploadNow = true);
        void SetVec4(const std::string& name, const float v[4], bool uploadNow = true);

        void SetMat3(const std::string& name, const float* m3x3, bool uploadNow = true); // 9 floats
        void SetMat4(const std::string& name, const float* m4x4, bool uploadNow = true); // 16 floats

        // If you do multiple Set() calls per frame, call Upload() once at end.
        void Upload();

    private:
        void Create(uint32_t size, bool dynamic);
        void Destroy();

        void WriteBytes(uint32_t offset, const void* data, uint32_t size);

    private:
        GLuint   m_RendererID = 0;
        uint32_t m_Size = 0;
        uint32_t m_BindingPoint = 0;

        UniformBufferLayout m_Layout;

        // CPU mirror for structured updates
        std::vector<std::uint8_t> m_CPU;
        bool m_Dirty = false;
    };
}
