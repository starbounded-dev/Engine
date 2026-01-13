#include "Mesh.h"
#include <cassert>

namespace Core::Renderer
{
    Mesh::Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, uint32_t materialIndex)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices)), m_MaterialIndex(materialIndex)
    {
        Build();
    }

    Mesh::~Mesh()
    {
        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }

    Mesh::Mesh(Mesh&& other) noexcept { *this = std::move(other); }

    Mesh& Mesh::operator=(Mesh&& other) noexcept
    {
        if (this == &other) return *this;

        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);

        m_Vertices = std::move(other.m_Vertices);
        m_Indices  = std::move(other.m_Indices);
        m_MaterialIndex = other.m_MaterialIndex;

        m_VAO = other.m_VAO;
        m_VB = std::move(other.m_VB);
        m_IB = std::move(other.m_IB);

        other.m_VAO = 0;
        other.m_MaterialIndex = 0;

        return *this;
    }

    void Mesh::Build()
    {
        assert(!m_Vertices.empty());
        assert(!m_Indices.empty());

        glCreateVertexArrays(1, &m_VAO);

        m_VB = VertexBuffer(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(MeshVertex)), BufferUsage::Static);
        m_VB.SetLayout({
            { ShaderDataType::Float3, "a_Position"  },
            { ShaderDataType::Float3, "a_Normal"    },
            { ShaderDataType::Float2, "a_TexCoord"  },
            { ShaderDataType::Float3, "a_Tangent"   },
            { ShaderDataType::Float3, "a_Bitangent" },
        });

        m_IB = IndexBuffer(m_Indices.data(), (uint32_t)m_Indices.size(), true, BufferUsage::Static);

        uint32_t attribLoc = 0;
        m_VB.BindToVertexArray(m_VAO, 0, attribLoc);
        m_IB.BindToVertexArray(m_VAO);
    }

    void Mesh::Draw() const
    {
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)m_IB.GetCount(), m_IB.GetIndexType(), nullptr);
    }
}
