#pragma once

#include "Event.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <string>
#include <functional>

namespace Core {

	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool IsResizeable = true;
		bool VSync = false;

		using EventCallbackFn = std::function<void(Event&)>;
		EventCallbackFn EventCallback;
	};

	class Window
	{
	public:
		Window(const WindowSpecification& specification = WindowSpecification());
		~Window();

		void Create();
		void Destroy();

		void Update();

		void RaiseEvent(Event& event);

		glm::vec2 GetFramebufferSize() const;
		glm::vec2 GetMousePos() const;

		bool ShouldClose() const;

		GLFWwindow* GetHandle() const { return m_Handle; }

		unsigned int GetWidth() const { return m_Specification.Width; }
		unsigned int GetHeight() const { return m_Specification.Height; }

		void Maximize();
		void CenterWindow();
	private:
		WindowSpecification m_Specification;

		GLFWwindow* m_Handle = nullptr;

	};

}