#include "OverlayLayer.h"

#include "Core/Application.h"

#include "Core/Renderer/Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AppLayer.h"
#include "VoidLayer.h"

#include <print>

OverlayLayer::OverlayLayer()
{
	std::println("Created new OverlayLayer!");

	// Create shaders
	m_Shader = Renderer::CreateGraphicsShader("Shaders/Transform.vert.glsl", "Shaders/Texture.frag.glsl");

	// Create geometry
	glCreateVertexArrays(1, &m_VertexArray);
	glCreateBuffers(1, &m_VertexBuffer);
	glCreateBuffers(1, &m_IndexBuffer);

	struct Vertex
	{
		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

	Vertex vertices[] = {
		{ {-0.5f, -0.5f }, { 0.0f, 0.0f } }, // Bottom-left
		{ { 0.5f, -0.5f }, { 1.0f, 0.0f } }, // Bottom-right
		{ { 0.5f,  0.5f }, { 1.0f, 1.0f } }, // Top-right
		{ {-0.5f,  0.5f }, { 0.0f, 1.0f } }  // Top-left
	};

	glNamedBufferData(m_VertexBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);

	uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
	glNamedBufferData(m_IndexBuffer, sizeof(indices), indices, GL_STATIC_DRAW);

	// Bind the VBO to VAO at binding index 0
	glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);

	// Enable attributes
	glEnableVertexArrayAttrib(m_VertexArray, 0); // position
	glEnableVertexArrayAttrib(m_VertexArray, 1); // uv

	// Format: location, size, type, normalized, relative offset
	glVertexArrayAttribFormat(m_VertexArray, 0, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, Position)));
	glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, static_cast<GLuint>(offsetof(Vertex, TexCoord)));

	// Link attribute locations to binding index 0
	glVertexArrayAttribBinding(m_VertexArray, 0, 0);
	glVertexArrayAttribBinding(m_VertexArray, 1, 0);

	m_Texture = Renderer::LoadTexture("Textures/Button.png");
}

OverlayLayer::~OverlayLayer()
{
	glDeleteVertexArrays(1, &m_VertexArray);
	glDeleteBuffers(1, &m_VertexBuffer);

	glDeleteProgram(m_Shader);

	glDeleteTextures(1, &m_Texture.Handle);
}

void OverlayLayer::OnEvent(Core::Event& event)
{
	Core::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Core::MouseButtonPressedEvent>([this](Core::MouseButtonPressedEvent& e) { return OnMouseButtonPressed(e); });
}

void OverlayLayer::OnUpdate(float ts)
{
	m_IsHovered = IsButtonHovered();

#if OLD
	// Transition layer when button clicked
	if (m_IsHovered && m_Pressed && glfwGetMouseButton(Core::Application::Get().GetWindow()->GetHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		auto voidLayer = Core::Application::Get().GetLayer<VoidLayer>();
		if (voidLayer)
		{
			voidLayer->TransitionTo<AppLayer>();
		}
		else
		{
			auto appLayer = Core::Application::Get().GetLayer<AppLayer>();
			appLayer->TransitionTo<VoidLayer>();
		}
	}

	m_Pressed = m_IsHovered && glfwGetMouseButton(Core::Application::Get().GetWindow()->GetHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
#endif
}

void OverlayLayer::OnRender()
{
	glUseProgram(m_Shader);

	// Uniforms
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, -0.75f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2604f, 0.2222f, 1.0f));
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(transform));
	glUniform1i(1, 0); // Texture
	glUniform1i(2, (int)m_IsHovered);

	glBindTextureUnit(0, m_Texture.Handle);

	glm::vec2 framebufferSize = Core::Application::Get().GetFramebufferSize();
	glViewport(0, 0, static_cast<GLsizei>(framebufferSize.x), static_cast<GLsizei>(framebufferSize.y));

	// Render
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(m_VertexArray);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

bool OverlayLayer::IsButtonHovered() const
{
	glm::vec2 framebufferSize = Core::Application::Get().GetFramebufferSize();
	glm::vec2 mousePos = Core::Application::Get().GetWindow()->GetMousePos();
	glm::vec2 normalizedMousePos = (mousePos / framebufferSize) * 2.0f - 1.0f;
	normalizedMousePos.y *= -1.0f;

	return normalizedMousePos.x > (-0.8f - 0.2604 * 0.5f) && normalizedMousePos.x < (-0.8f + 0.2604f * 0.5f)
		&& normalizedMousePos.y >(-0.75f - 0.2222f * 0.5f) && normalizedMousePos.y < (-0.75f + 0.2222f * 0.5f);
}

bool OverlayLayer::OnMouseButtonPressed(Core::MouseButtonPressedEvent& event)
{
	if (!IsButtonHovered())
		return false;

	auto voidLayer = Core::Application::Get().GetLayer<VoidLayer>();
	if (voidLayer)
	{
		voidLayer->TransitionTo<AppLayer>();
	}
	else
	{
		auto appLayer = Core::Application::Get().GetLayer<AppLayer>();
		//appLayer->TransitionTo<VoidLayer>();
	}

	return true;
}
