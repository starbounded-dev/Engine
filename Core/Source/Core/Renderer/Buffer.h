#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <initializer_list>

#include <glad/glad.h>

namespace Core::Renderer
{
    enum class BufferUsage : uint8_t
    {
        Static = 0,
        Dynamic
    };

    enum class ShaderDataType : uint8_t
    {
        None = 0,

        // Floats
        Float, Float2, Float3, Float4,
        Mat3, Mat4,

        // Ints
        Int, Int2, Int3, Int4,

        // UInts
        UInt, UInt2, UInt3, UInt4,

        // Bool
        Bool
    };

    static uint32_t ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Mat3:   return 4 * 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4 * 4;

        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Int2:   return 4 * 2;
        case ShaderDataType::Int3:   return 4 * 3;
        case ShaderDataType::Int4:   return 4 * 4;

        case ShaderDataType::UInt:   return 4;
        case ShaderDataType::UInt2:  return 4 * 2;
        case ShaderDataType::UInt3:  return 4 * 3;
        case ShaderDataType::UInt4:  return 4 * 4;

        case ShaderDataType::Bool:   return 1;
        default: return 0;
        }
    }

    static uint32_t ShaderDataTypeComponentCount(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:  return 1;
        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Float4: return 4;
        case ShaderDataType::Mat3:   return 3; // 3 columns
        case ShaderDataType::Mat4:   return 4; // 4 columns

        case ShaderDataType::Int:    return 1;
        case ShaderDataType::Int2:   return 2;
        case ShaderDataType::Int3:   return 3;
        case ShaderDataType::Int4:   return 4;

        case ShaderDataType::UInt:   return 1;
        case ShaderDataType::UInt2:  return 2;
        case ShaderDataType::UInt3:  return 3;
        case ShaderDataType::UInt4:  return 4;

        case ShaderDataType::Bool:   return 1;
        default: return 0;
        }
    }

    static bool ShaderDataTypeIsInteger(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4:
        case ShaderDataType::UInt:
        case ShaderDataType::UInt2:
        case ShaderDataType::UInt3:
        case ShaderDataType::UInt4:
        case ShaderDataType::Bool:
            return true;
        default:
            return false;
        }
    }

    static bool ShaderDataTypeIsMatrix(ShaderDataType type)
    {
        return type == ShaderDataType::Mat3 || type == ShaderDataType::Mat4;
    }

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4:
            return GL_FLOAT;

        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4:
            return GL_INT;

        case ShaderDataType::UInt:
        case ShaderDataType::UInt2:
        case ShaderDataType::UInt3:
        case ShaderDataType::UInt4:
            return GL_UNSIGNED_INT;

        case ShaderDataType::Bool:
            return GL_BOOL;

        default:
            return GL_FLOAT;
        }
    }

    struct BufferElement
    {
        std::string    Name;
        ShaderDataType Type = ShaderDataType::None;
        uint32_t       Size = 0;
        uint32_t       Offset = 0;
        bool           Normalized = false;

        BufferElement() = default;

        BufferElement(ShaderDataType type, std::string name, bool normalized = false)
            : Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Normalized(normalized)
        {
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() = default;
        VertexBufferLayout(std::initializer_list<BufferElement> elements)
            : m_Elements(elements)
        {
            CalculateOffsetsAndStride();
        }

        uint32_t GetStride() const { return m_Stride; }
        const std::vector<BufferElement>& GetElements() const { return m_Elements; }

        std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
        std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
        std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
        std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

    private:
        void CalculateOffsetsAndStride()
        {
            uint32_t offset = 0;
            m_Stride = 0;
            for (auto& e : m_Elements)
            {
                e.Offset = offset;
                offset += e.Size;
                m_Stride += e.Size;
            }
        }

    private:
        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride = 0;
    };

    class VertexBuffer
    {
    public:
        VertexBuffer() = default;
        VertexBuffer(const void* data, uint32_t size, BufferUsage usage = BufferUsage::Static);
        VertexBuffer(uint32_t size, BufferUsage usage = BufferUsage::Dynamic);
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;

        GLuint GetRendererID() const { return m_RendererID; }
        uint32_t GetSize() const { return m_Size; }
        BufferUsage GetUsage() const { return m_Usage; }

        void SetLayout(const VertexBufferLayout& layout) { m_Layout = layout; }
        const VertexBufferLayout& GetLayout() const { return m_Layout; }

        // Updates GPU data (works best for Dynamic, but allowed for both)
        void SetData(const void* data, uint32_t size, uint32_t offset = 0);

        // For classic binding usage (optional)
        void Bind() const;
        void Unbind() const;

        // DSA: hook this VB + layout into a VAO automatically.
        // attribLocation is in/out so you can chain multiple VBs without collisions.
        void BindToVertexArray(GLuint vao, uint32_t bindingIndex, uint32_t& attribLocation) const;

    private:
        void Create(const void* data, uint32_t size, BufferUsage usage);
        void Destroy();

    private:
        GLuint m_RendererID = 0;
        uint32_t m_Size = 0;
        BufferUsage m_Usage = BufferUsage::Static;
        VertexBufferLayout m_Layout;
    };

    class IndexBuffer
    {
    public:
        IndexBuffer() = default;
        IndexBuffer(const void* indices, uint32_t count, bool use32Bit = true, BufferUsage usage = BufferUsage::Static);
        ~IndexBuffer();

        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        GLuint GetRendererID() const { return m_RendererID; }
        uint32_t GetCount() const { return m_Count; }
        GLenum GetIndexType() const { return m_IndexType; }
        uint32_t GetIndexSize() const { return (m_IndexType == GL_UNSIGNED_INT) ? 4u : 2u; }

        void SetData(const void* indices, uint32_t count, uint32_t offsetBytes = 0);

        void Bind() const;
        void Unbind() const;

        // DSA: attach as element buffer for a VAO
        void BindToVertexArray(GLuint vao) const;

    private:
        void Create(const void* indices, uint32_t count, bool use32Bit, BufferUsage usage);
        void Destroy();

    private:
        GLuint m_RendererID = 0;
        uint32_t m_Count = 0;
        GLenum m_IndexType = GL_UNSIGNED_INT;
        BufferUsage m_Usage = BufferUsage::Static;
    };
}
