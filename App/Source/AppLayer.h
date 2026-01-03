#pragma once

#include "Core/Layer.h"
#include "Core/InputEvents.h"
#include "Core/WindowEvents.h"

#include <glm/glm.hpp>

#include <stdint.h>

class AppLayer : public Core::Layer
{
public:
	AppLayer();
	virtual ~AppLayer();

	virtual void OnEvent(Core::Event& event) override;

	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
private:
	bool OnMouseButtonPressed(Core::MouseButtonPressedEvent& event);
	bool OnMouseMoved(Core::MouseMovedEvent& event);
	bool OnWindowClosed(Core::WindowClosedEvent& event);
private:
	uint32_t m_Shader = 0;
	uint32_t m_VertexArray = 0;
	uint32_t m_VertexBuffer = 0;

	float m_Time = 0.0f;
	glm::vec2 m_MousePosition{ 0.0f };
	glm::vec2 m_FlamePosition{ 0.0f };
};
