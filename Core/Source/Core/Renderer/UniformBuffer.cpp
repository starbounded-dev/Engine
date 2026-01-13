#include "UniformBuffer.h"

#include <cstring>
#include <cassert>

namespace Core::Renderer
{
    const UniformBufferElement* UniformBufferLayout::Find(const std::string& name) const
    {
        auto it = m_Elements.find(name);
        if (it == m_Elements.end())
            return nullptr;
        return &it->second;
    }

    UniformBufferLayout UniformBufferLayout::Reflect(GLuint program, const std::string& blockName)
    {
        UniformBufferLayout layout;

        GLuint blockIndex = glGetUniformBlockIndex(program, blockName.c_str());
        if (blockIndex == GL_INVALID_INDEX)
        {
            // Block doesn't exist in this shader.
            return layout;
        }

        GLint blockSize = 0;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        layout.m_Size = (blockSize > 0) ? (uint32_t)blockSize : 0;

        GLint activeUniformCount = 0;
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);
        if (activeUniformCount <= 0 || layout.m_Size == 0)
            return layout;

        std::vector<GLint> uniformIndices(activeUniformCount);
        glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices.data());

        std::vector<GLint> offsets(activeUniformCount);
        std::vector<GLint> sizes(activeUniformCount);
        std::vector<GLint> types(activeUniformCount);
        std::vector<GLint> arrayStrides(activeUniformCount);
        std::vector<GLint> matrixStrides(activeUniformCount);

        glGetActiveUniformsiv(program, activeUniformCount, (GLuint*)uniformIndices.data(), GL_UNIFORM_OFFSET, offsets.data());
        glGetActiveUniformsiv(program, activeUniformCount, (GLuint*)uniformIndices.data(), GL_UNIFORM_SIZE, sizes.data());
        glGetActiveUniformsiv(program, activeUniformCount, (GLuint*)uniformIndices.data(), GL_UNIFORM_TYPE, types.data());
        glGetActiveUniformsiv(program, activeUniformCount, (GLuint*)uniformIndices.data(), GL_UNIFORM_ARRAY_STRIDE, arrayStrides.data());
        glGetActiveUniformsiv(program, activeUniformCount, (GLuint*)uniformIndices.data(), GL_UNIFORM_MATRIX_STRIDE, matrixStrides.data());

        for (int i = 0; i < activeUniformCount; i++)
        {
            GLchar nameBuf[256];
            GLsizei nameLen = 0;
            GLint   dummySize = 0;
            GLenum  dummyType = 0;

            glGetActiveUniform(program, (GLuint)uniformIndices[i], (GLsizei)sizeof(nameBuf), &nameLen, &dummySize, &dummyType, nameBuf);
            std::string uname(nameBuf, (size_t)nameLen);

            // Some drivers return "BlockName.member", some return "member" depending on usage.
            // Keep it as-is; you can normalize in your Material if you want.
            UniformBufferElement e;
            e.Name = uname;
            e.Offset = (uint32_t)offsets[i];
            e.ArrayStride = (uint32_t)arrayStrides[i];
            e.MatrixStride = (uint32_t)matrixStrides[i];
            e.Type = (GLenum)types[i];

            // Size in bytes isn't directly given for every type; we’ll compute a safe byte-size.
            // For std140 use offsets + nextOffset; but we don't have nextOffset reliably here.
            // We'll store a conservative "component byte size" based on type * uniform size.
            // In practice, we write exact bytes for float/int/vec/mat with known sizes.
            e.Size = 0;

            layout.m_Elements[e.Name] = e;
        }

        return layout;
    }

    // -------------------- UniformBuffer --------------------

    UniformBuffer::UniformBuffer(uint32_t size, uint32_t bindingPoint, bool dynamic)
        : m_Size(size), m_BindingPoint(bindingPoint)
    {
        Create(size, dynamic);
        m_CPU.resize(size);
        BindBase();
    }

    UniformBuffer::UniformBuffer(const UniformBufferLayout& layout, uint32_t bindingPoint, bool dynamic)
        : m_Size(layout.GetSize()), m_BindingPoint(bindingPoint), m_Layout(layout)
    {
        Create(m_Size, dynamic);
        m_CPU.resize(m_Size);
        BindBase();
    }

    UniformBuffer::~UniformBuffer()
    {
        Destroy();
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    {
        *this = std::move(other);
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
    {
        if (this == &other)
            return *this;

        Destroy();

        m_RendererID = other.m_RendererID;
        m_Size = other.m_Size;
        m_BindingPoint = other.m_BindingPoint;
        m_Layout = std::move(other.m_Layout);
        m_CPU = std::move(other.m_CPU);
        m_Dirty = other.m_Dirty;

        other.m_RendererID = 0;
        other.m_Size = 0;
        other.m_BindingPoint = 0;
        other.m_Dirty = false;

        return *this;
    }

    void UniformBuffer::Create(uint32_t size, bool dynamic)
    {
        assert(size > 0);

        glCreateBuffers(1, &m_RendererID);

        // DSA storage
        GLbitfield flags = 0;
        if (dynamic)
            flags |= GL_DYNAMIC_STORAGE_BIT;

        glNamedBufferStorage(m_RendererID, (GLsizeiptr)size, nullptr, flags);
    }

    void UniformBuffer::Destroy()
    {
        if (m_RendererID)
        {
            glDeleteBuffers(1, &m_RendererID);
            m_RendererID = 0;
        }
    }

    void UniformBuffer::BindBase() const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID);
    }

    void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        assert(m_RendererID);
        assert(offset + size <= m_Size);

        glNamedBufferSubData(m_RendererID, (GLintptr)offset, (GLsizeiptr)size, data);
    }

    void UniformBuffer::WriteBytes(uint32_t offset, const void* data, uint32_t size)
    {
        assert(offset + size <= m_Size);
        std::memcpy(m_CPU.data() + offset, data, size);
        m_Dirty = true;
    }

    void UniformBuffer::Upload()
    {
        if (!m_Dirty)
            return;

        glNamedBufferSubData(m_RendererID, 0, (GLsizeiptr)m_Size, m_CPU.data());
        m_Dirty = false;
    }

    // -------- structured setters (write into CPU mirror, then Upload) --------

    void UniformBuffer::SetFloat(const std::string& name, float v, bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, &v, sizeof(float));
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetInt(const std::string& name, int v, bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, &v, sizeof(int));
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetUInt(const std::string& name, uint32_t v, bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, &v, sizeof(uint32_t));
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetVec2(const std::string& name, const float v[2], bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, v, sizeof(float) * 2);
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetVec3(const std::string& name, const float v[3], bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, v, sizeof(float) * 3);
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetVec4(const std::string& name, const float v[4], bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, v, sizeof(float) * 4);
        if (uploadNow) Upload();
    }

    void UniformBuffer::SetMat3(const std::string& name, const float* m3x3, bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");

        // NOTE: std140 mat3 is typically 3 vec4 columns (stride 16) => 48 bytes.
        // If you declare mat3 in std140, reflection gives MatrixStride.
        if (e->MatrixStride > 0)
        {
            // write column by column respecting stride
            for (int col = 0; col < 3; col++)
                WriteBytes(e->Offset + col * e->MatrixStride, m3x3 + col * 3, sizeof(float) * 3);
        }
        else
        {
            WriteBytes(e->Offset, m3x3, sizeof(float) * 9);
        }

        if (uploadNow) Upload();
    }

    void UniformBuffer::SetMat4(const std::string& name, const float* m4x4, bool uploadNow)
    {
        auto e = m_Layout.Find(name);
        assert(e && "Uniform not found in UBO layout");
        WriteBytes(e->Offset, m4x4, sizeof(float) * 16);
        if (uploadNow) Upload();
    }
}
