#include "Buffer.h"
#include <cassert>
#include <cstring>

namespace Core::Renderer
{
    // ---------------- VertexBuffer ----------------

    VertexBuffer::VertexBuffer(const void* data, uint32_t size, BufferUsage usage)
    {
        Create(data, size, usage);
    }

    VertexBuffer::VertexBuffer(uint32_t size, BufferUsage usage)
    {
        Create(nullptr, size, usage);
    }

    VertexBuffer::~VertexBuffer()
    {
        Destroy();
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    {
        *this = std::move(other);
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
    {
        if (this == &other) return *this;

        Destroy();

        m_RendererID = other.m_RendererID;
        m_Size = other.m_Size;
        m_Usage = other.m_Usage;
        m_Layout = std::move(other.m_Layout);

        other.m_RendererID = 0;
        other.m_Size = 0;

        return *this;
    }

    void VertexBuffer::Create(const void* data, uint32_t size, BufferUsage usage)
    {
        assert(size > 0);

        m_Size = size;
        m_Usage = usage;

        glCreateBuffers(1, &m_RendererID);

        GLbitfield flags = 0;
        if (usage == BufferUsage::Dynamic)
            flags |= GL_DYNAMIC_STORAGE_BIT;

        glNamedBufferStorage(m_RendererID, (GLsizeiptr)size, data, flags);
    }

    void VertexBuffer::Destroy()
    {
        if (m_RendererID)
        {
            glDeleteBuffers(1, &m_RendererID);
            m_RendererID = 0;
        }
    }

    void VertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        assert(m_RendererID);
        assert(offset + size <= m_Size);
        glNamedBufferSubData(m_RendererID, (GLintptr)offset, (GLsizeiptr)size, data);
    }

    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::BindToVertexArray(GLuint vao, uint32_t bindingIndex, uint32_t& attribLocation) const
    {
        assert(vao != 0);
        assert(m_RendererID != 0);

        const auto& layout = m_Layout;
        assert(layout.GetStride() > 0 && "VertexBuffer has no layout!");

        // Attach VB to VAO binding slot
        glVertexArrayVertexBuffer(vao, bindingIndex, m_RendererID, 0, (GLsizei)layout.GetStride());

        for (const auto& e : layout.GetElements())
        {
            if (!ShaderDataTypeIsMatrix(e.Type))
            {
                const GLenum baseType = ShaderDataTypeToOpenGLBaseType(e.Type);
                const uint32_t count = ShaderDataTypeComponentCount(e.Type);

                glEnableVertexArrayAttrib(vao, attribLocation);

                if (ShaderDataTypeIsInteger(e.Type))
                {
                    glVertexArrayAttribIFormat(vao, attribLocation, (GLint)count, baseType, (GLuint)e.Offset);
                }
                else
                {
                    glVertexArrayAttribFormat(vao, attribLocation, (GLint)count, baseType,
                        e.Normalized ? GL_TRUE : GL_FALSE, (GLuint)e.Offset);
                }

                glVertexArrayAttribBinding(vao, attribLocation, bindingIndex);
                attribLocation++;
            }
            else
            {
                // Mat3 = 3 vec3 columns, Mat4 = 4 vec4 columns
                const GLenum baseType = GL_FLOAT;
                const uint32_t cols = ShaderDataTypeComponentCount(e.Type);
                const uint32_t rows = (e.Type == ShaderDataType::Mat3) ? 3u : 4u;

                const uint32_t columnSizeBytes = rows * sizeof(float);

                for (uint32_t c = 0; c < cols; c++)
                {
                    glEnableVertexArrayAttrib(vao, attribLocation);
                    glVertexArrayAttribFormat(vao, attribLocation, (GLint)rows, baseType, GL_FALSE,
                        (GLuint)(e.Offset + c * columnSizeBytes));
                    glVertexArrayAttribBinding(vao, attribLocation, bindingIndex);

                    // NOTE: If you ever want instanced matrices, you would set:
                    // glVertexArrayBindingDivisor(vao, bindingIndex, 1);
                    attribLocation++;
                }
            }
        }
    }

    // ---------------- IndexBuffer ----------------

    IndexBuffer::IndexBuffer(const void* indices, uint32_t count, bool use32Bit, BufferUsage usage)
    {
        Create(indices, count, use32Bit, usage);
    }

    IndexBuffer::~IndexBuffer()
    {
        Destroy();
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    {
        *this = std::move(other);
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
    {
        if (this == &other) return *this;

        Destroy();

        m_RendererID = other.m_RendererID;
        m_Count = other.m_Count;
        m_IndexType = other.m_IndexType;
        m_Usage = other.m_Usage;

        other.m_RendererID = 0;
        other.m_Count = 0;

        return *this;
    }

    void IndexBuffer::Create(const void* indices, uint32_t count, bool use32Bit, BufferUsage usage)
    {
        assert(count > 0);

        m_Count = count;
        m_Usage = usage;
        m_IndexType = use32Bit ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

        const uint32_t byteSize = count * (use32Bit ? 4u : 2u);

        glCreateBuffers(1, &m_RendererID);

        GLbitfield flags = 0;
        if (usage == BufferUsage::Dynamic)
            flags |= GL_DYNAMIC_STORAGE_BIT;

        glNamedBufferStorage(m_RendererID, (GLsizeiptr)byteSize, indices, flags);
    }

    void IndexBuffer::Destroy()
    {
        if (m_RendererID)
        {
            glDeleteBuffers(1, &m_RendererID);
            m_RendererID = 0;
        }
    }

    void IndexBuffer::SetData(const void* indices, uint32_t count, uint32_t offsetBytes)
    {
        assert(m_RendererID);
        const uint32_t byteSize = count * ((m_IndexType == GL_UNSIGNED_INT) ? 4u : 2u);
        glNamedBufferSubData(m_RendererID, (GLintptr)offsetBytes, (GLsizeiptr)byteSize, indices);

        // If you replace the whole buffer, update count
        if (offsetBytes == 0)
            m_Count = count;
    }

    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void IndexBuffer::BindToVertexArray(GLuint vao) const
    {
        assert(vao != 0);
        glVertexArrayElementBuffer(vao, m_RendererID);
    }
}
