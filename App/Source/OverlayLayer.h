#pragma once

#include <stdint.h>

#include "Core/Layer.h"
#include "Core/InputEvents.h"

#include "Core/Renderer/Renderer.h"

class OverlayLayer : public Core::Layer
{
public:
	OverlayLayer();
	virtual ~OverlayLayer();

	virtual void OnEvent(Core::Event& event) override;

	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
private:
	bool IsButtonHovered() const;

	bool OnMouseButtonPressed(Core::MouseButtonPressedEvent& event);
private:
	uint32_t m_Shader = 0;
	uint32_t m_VertexArray = 0;
	uint32_t m_VertexBuffer = 0;
	uint32_t m_IndexBuffer = 0;
	Renderer::Texture m_Texture;

	bool m_IsHovered = false;
	bool m_Pressed = true;
};