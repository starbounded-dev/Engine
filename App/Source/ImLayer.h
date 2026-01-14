#pragma once

#include "Core/Layer.h"
#include "Core/InputEvents.h"
#include "Editor/ProfilerPanel.h"
#include "Editor/ShaderEditor.h"
#include "Editor/StatsPanel.h"
#include "Editor/MaterialEditor.h"
#include "Editor/ModelPanel.h"
#include <memory>

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

		void RenderMenuBar();
		void RenderPanels();
		void OnOverlayRender();
		void UpdateProfilerMetrics();
		void ApplyCustomStyle();

	private:
		// Window visibility flags
		bool m_ShowDemoWindow = false;
		bool m_ShowOverlay = true;
		bool m_ShowProfiler = true;
		bool m_ShowShaderEditor = true;
		bool m_ShowViewport = true;
		bool m_ShowStats = true;
		bool m_ShowStatsPanel = true;
		bool m_ShowMaterialEditor = false;
		bool m_ShowModelPanel = false;
		
		// Editor panels
		std::unique_ptr<Editor::ProfilerPanel> m_ProfilerPanel;
		std::unique_ptr<Editor::ShaderEditor> m_ShaderEditor;
		std::unique_ptr<Editor::StatsPanel> m_StatsPanel;
		std::unique_ptr<Editor::MaterialEditor> m_MaterialEditor;
		std::unique_ptr<Editor::ModelPanel> m_ModelPanel;
		
		// Stats
		int m_Clicks = 0;
		float m_LastFrameTime = 0.0f;
	};
}
