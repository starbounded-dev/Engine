#pragma once

#include "Core/Layer.h"
#include "Core/InputEvents.h"

namespace Core
{
	class ImLayer : public Layer
	{
	public:
		ImLayer();
		virtual ~ImLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnOverlayRender();

	private:
		bool m_ShowDemoWindow = true;
		bool m_ShowOverlay = true;
		int  m_Clicks = 0;
	};
}
