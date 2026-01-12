#pragma once

#include "Core/Layer.h"

#include "Core/Event.h"

namespace Core {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();
		void SetDarkThemeV2Colors();

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_BlockEvents = true;
	};

}
