#include "Core/Layer.h"
#include "Core/Input/Input.h"
#include "Core/Application.h"
#include "Core/Renderer/Material.h"
#include "Core/Renderer/Buffer.h"

class ExampleLayer : public Core::Layer
{
private:
    Core::Renderer::Material m_Material;
    Core::Renderer::VertexBuffer m_VertexBuffer;
    Core::Renderer::IndexBuffer  m_IndexBuffer;

    GLuint m_VertexArray = 0;
    float m_Time = 0.0f;

public:
    ExampleLayer()
    {
        // Material (new shaders)
        m_Material = Core::Renderer::Material(
            "Resources/Shaders/FullscreenQuad.vert.glsl",
            "Resources/Shaders/ProceduralFlame.frag.glsl"
        );

        struct Vertex { glm::vec2 pos; glm::vec2 uv; };
        Vertex vertices[] = {
            { {-1.0f, -1.0f}, {0.0f, 0.0f} },
            { { 1.0f, -1.0f}, {1.0f, 0.0f} },
            { { 1.0f,  1.0f}, {1.0f, 1.0f} },
            { {-1.0f,  1.0f}, {0.0f, 1.0f} }
        };

        uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

        m_VertexBuffer = Core::Renderer::VertexBuffer(vertices, sizeof(vertices), Core::Renderer::BufferUsage::Static);
        m_IndexBuffer  = Core::Renderer::IndexBuffer(indices, 6, Core::Renderer::BufferUsage::Static);

        // VAO (DSA)
        glCreateVertexArrays(1, &m_VertexArray);

        // NOTE: adapt GetRendererID() to whatever your wrapper exposes
        GLuint vb = m_VertexBuffer.GetHandle();
        GLuint ib = m_IndexBuffer.GetHandle();

        glVertexArrayVertexBuffer(m_VertexArray, 0, vb, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(m_VertexArray, ib);

        // a_Position (location=0)
        glEnableVertexArrayAttrib(m_VertexArray, 0);
        glVertexArrayAttribFormat(m_VertexArray, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
        glVertexArrayAttribBinding(m_VertexArray, 0, 0);

        // a_TexCoord (location=1)
        glEnableVertexArrayAttrib(m_VertexArray, 1);
        glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
        glVertexArrayAttribBinding(m_VertexArray, 1, 0);
    }

    void OnUpdate(float ts) override
    {
        m_Time += ts;

        if (Core::Input::IsKeyPressed(Core::Key::Escape))
            Core::Application::Get().Stop();
    }

    void OnRender() override
    {
        auto size = Core::Application::Get().GetFramebufferSize();

        m_Material.Bind();
        m_Material.SetFloat("u_Time", m_Time);

        // If you don't have SetVec2 yet, you'll need to add it in Material.
        m_Material.SetFloat2("u_Resolution", { size.x, size.y });
        m_Material.SetFloat2("u_FlameOrigin", { 0.5f, 0.05f });

        glBindVertexArray(m_VertexArray);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
};