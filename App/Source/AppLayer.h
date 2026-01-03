#pragma once

#include <stdint.h>

#include "Core/Layer.h"

class AppLayer : public Core::Layer
{
public:
	AppLayer();
	virtual ~AppLayer();

	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
private:
	uint32_t m_Shader = 0;
	uint32_t m_VertexArray = 0;
	uint32_t m_VertexBuffer = 0;
};
