#pragma once
#include <vector>
#include <cstdint>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Core/Renderer/Buffer.h"

namespace Core::Renderer
{
    struct MeshVertex
    {
        glm::vec3 Position{0.0f};
        glm::vec3 Normal{0.0f, 1.0f, 0.0f};
        glm::vec2 TexCoord{0.0f};
        glm::vec3 Tangent{0.0f};
        glm::vec3 Bitangent{0.0f};
    };

    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, uint32_t materialIndex = 0);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void Draw() const;

        uint32_t GetMaterialIndex() const { return m_MaterialIndex; }
        const std::vector<MeshVertex>& GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

        GLuint GetVAO() const { return m_VAO; }

    private:
        void Build();

    private:
        std::vector<MeshVertex> m_Vertices;
        std::vector<uint32_t>   m_Indices;
        uint32_t m_MaterialIndex = 0;

        GLuint m_VAO = 0;
        VertexBuffer m_VB;
        IndexBuffer  m_IB;
    };
}
