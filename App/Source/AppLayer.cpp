#include "AppLayer.h"

#include "Core/Application.h"

#include "Core/Renderer/Renderer.h"
#include "Core/Renderer/Shader.h"

#include <glm/glm.hpp>

AppLayer::AppLayer()
{
	// Create shaders
	m_Shader = Renderer::CreateGraphicsShader("Resources/Shaders/Vertex.glsl", "Resources/Shaders/Fragment.glsl");

	// Create geometry
	glCreateVertexArrays(1, &m_VertexArray);
	glCreateBuffers(1, &m_VertexBuffer);

	struct Vertex
	{
		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

	Vertex vertices[] = {
		{ {-1.0f, -1.0f }, { 0.0f, 0.0f } },  // Bottom-left
		{ { 3.0f, -1.0f }, { 2.0f, 0.0f } },  // Bottom-right
		{ {-1.0f,  3.0f }, { 0.0f, 2.0f } }   // Top-left
	};

	glNamedBufferData(m_VertexBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind the VBO to VAO at binding index 0
	glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(Vertex));

	// Enable attributes
	glEnableVertexArrayAttrib(m_VertexArray, 0); // position
	glEnableVertexArrayAttrib(m_VertexArray, 1); // uv

	// Format: location, size, type, normalized, relative offset
	glVertexArrayAttribFormat(m_VertexArray, 0, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, Position)));
	glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, TexCoord)));

	// Link attribute locations to binding index 0
	glVertexArrayAttribBinding(m_VertexArray, 0, 0);
	glVertexArrayAttribBinding(m_VertexArray, 1, 0);
}

AppLayer::~AppLayer()
{
	glDeleteVertexArrays(1, &m_VertexArray);
	glDeleteBuffers(1, &m_VertexBuffer);

	glDeleteProgram(m_Shader);
}

void AppLayer::OnUpdate(float ts)
{
}

void AppLayer::OnRender()
{
	glUseProgram(m_Shader);

	// Uniforms
	glUniform1f(0, Core::Application::GetTime());

	glm::vec2 framebufferSize = Core::Application::Get().GetFramebufferSize();
	glUniform2f(1, framebufferSize.x, framebufferSize.y);

	glViewport(0, 0, static_cast<GLsizei>(framebufferSize.x), static_cast<GLsizei>(framebufferSize.y));

	// Render
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(m_VertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
