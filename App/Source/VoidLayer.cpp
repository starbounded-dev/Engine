#include "VoidLayer.h"

#include "AppLayer.h"

#include "Core/Application.h"
#include "Core/Renderer/Renderer.h"

void VoidLayer::OnUpdate(float ts)
{
	if (glfwGetKey(Core::Application::Get().GetWindow()->GetHandle(), GLFW_KEY_2) == GLFW_PRESS)
	{
		TransitionTo<AppLayer>();
	}
}

void VoidLayer::OnRender()
{
	glClearColor(0.6f, 0.1f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
