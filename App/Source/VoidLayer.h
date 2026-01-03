#pragma once

#include "Core/Layer.h"

class VoidLayer : public Core::Layer
{
public:
	VoidLayer() {}
	virtual ~VoidLayer() {}

	virtual void OnUpdate(float ts) override;
	virtual void OnRender() override;
};