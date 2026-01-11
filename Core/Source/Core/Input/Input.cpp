#include "Input.h"

#include "Core/Application.h"
#include "Core/Debug/Profiler.h"

#include <GLFW/glfw3.h>

namespace Core {

	void* Input::s_Window = nullptr;

	void Input::SetWindow(void* window)
	{
		s_Window = window;
	}

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		PROFILE_FUNC();

		if (!s_Window)
			return false;

		auto state = glfwGetKey(static_cast<GLFWwindow*>(s_Window), keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsKeyReleased(KeyCode keycode)
	{
		PROFILE_FUNC();

		if (!s_Window)
			return false;

		auto state = glfwGetKey(static_cast<GLFWwindow*>(s_Window), keycode);
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		PROFILE_FUNC();

		if (!s_Window)
			return false;

		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(s_Window), button);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		PROFILE_FUNC();

		if (!s_Window)
			return false;

		auto state = glfwGetMouseButton(static_cast<GLFWwindow*>(s_Window), button);
		return state == GLFW_RELEASE;
	}

	glm::vec2 Input::GetMousePosition()
	{
		PROFILE_FUNC();

		if (!s_Window)
			return { 0.0f, 0.0f };

		double xpos, ypos;
		glfwGetCursorPos(static_cast<GLFWwindow*>(s_Window), &xpos, &ypos);
		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

	void Input::Update()
	{
		// Currently no per-frame state to update, but this can be used for key state tracking in the future
	}

}
