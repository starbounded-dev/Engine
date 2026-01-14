#include "ImLayer.h"

#include "Core/Application.h"
#include "Core/Renderer/Renderer.h"

#include "Core/Debug/Profiler.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "imgui_internal.h"
#include "glad/glad.h"

namespace Core
{
	ImLayer::ImLayer()
	: Layer("ImLayer")
		{

		}

	void ImLayer::OnAttach()
	{
		PROFILE_FUNC();

		// Initialize editor panels
		m_ProfilerPanel = std::make_unique<::Editor::ProfilerPanel>();
		m_ShaderEditor = std::make_unique<::Editor::ShaderEditor>();
		m_StatsPanel = std::make_unique<::Editor::StatsPanel>();
		m_MaterialEditor = std::make_unique<::Editor::MaterialEditor>();
		m_ModelPanel = std::make_unique<::Editor::ModelPanel>();
		
		// Register shader editor instance for global access
		::Editor::ShaderEditor::SetInstance(m_ShaderEditor.get());

		// Apply custom styling
		ApplyCustomStyle();
	}	

	void ImLayer::OnDetach()
	{
		PROFILE_FUNC();
		
		// Unregister shader editor instance
		::Editor::ShaderEditor::SetInstance(nullptr);

		// Cleanup panels
		m_ProfilerPanel.reset();
		m_ShaderEditor.reset();
		m_StatsPanel.reset();
		m_MaterialEditor.reset();
		m_ModelPanel.reset();
	}

	void ImLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& ev) { return OnKeyPressed(ev); });
		dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& ev) { return OnMouseButtonPressed(ev); });
	}

	void ImLayer::OnRender()
	{
		PROFILE_FUNC();

		auto size = Core::Application::Get().GetFramebufferSize();
		Renderer::BeginFrame((int)size.x, (int)size.y);

		// Track frame time for profiler
		auto& io = ImGui::GetIO();
		m_LastFrameTime = io.DeltaTime * 1000.0f; // Convert to milliseconds
	}

	void ImLayer::OnImGuiRender()
	{
		PROFILE_FUNC();

		static bool dockspaceOpen = true;
		static bool fullscreen = true;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		if (fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);

			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiID dockspaceID = ImGui::GetID("EngineDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

		RenderMenuBar();

		ImGui::End(); // DockSpace

		// Render all panels
		RenderPanels();

		// Update profiler metrics
		UpdateProfilerMetrics();

		if (m_ShowOverlay)
		OnOverlayRender();
	}

	void ImLayer::RenderMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit", "Alt+F4"))
					Core::Application::Get().Stop();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Viewport", nullptr, &m_ShowViewport);
				ImGui::MenuItem("Statistics", nullptr, &m_ShowStats);
				ImGui::Separator();
				ImGui::MenuItem("Profiler", "F3", &m_ShowProfiler);
				ImGui::MenuItem("Shader Editor", "F4", &m_ShowShaderEditor);
				ImGui::MenuItem("Renderer Stats", "F5", &m_ShowStatsPanel);
				ImGui::MenuItem("Material Editor", "F6", &m_ShowMaterialEditor);
				ImGui::MenuItem("Model Viewer", "F7", &m_ShowModelPanel);
				ImGui::Separator();
				ImGui::MenuItem("Overlay", "F2", &m_ShowOverlay);
				ImGui::MenuItem("ImGui Demo", "F1", &m_ShowDemoWindow);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Profiler", "F3", &m_ShowProfiler)) {}
				if (ImGui::MenuItem("Shader Editor", "F4", &m_ShowShaderEditor)) {}
				if (ImGui::MenuItem("Renderer Stats", "F5", &m_ShowStatsPanel)) {}
				if (ImGui::MenuItem("Material Editor", "F6", &m_ShowMaterialEditor)) {}
				if (ImGui::MenuItem("Model Viewer", "F7", &m_ShowModelPanel)) {}
					ImGui::Separator();
				if (ImGui::MenuItem("Reset Layout"))
				{
					// Reset docking layout
					ImGui::DockBuilderRemoveNode(ImGui::GetID("EngineDockSpace"));
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("About", nullptr, false, false);
				ImGui::Separator();
				ImGui::Text("Engine v1.0.0");
				ImGui::Text("Renderer: OpenGL 4.5+");
				ImGui::EndMenu();
			}

			// Right-aligned info
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 180);
			ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, m_LastFrameTime);

			ImGui::EndMenuBar();
		}
	}

	void ImLayer::RenderPanels()
	{
		PROFILE_FUNC();

		// Viewport Window
		if (m_ShowViewport)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Viewport", &m_ShowViewport);
			ImGui::PopStyleVar();

			ImVec2 viewportSize = ImGui::GetContentRegionAvail();

			// Viewport content would go here
			ImGui::Text("3D Viewport");
			ImGui::Text("Size: %.0f x %.0f", viewportSize.x, viewportSize.y);

			// Placeholder for viewport rendering
			ImGui::Dummy(ImVec2(viewportSize.x, viewportSize.y - 50));

			ImGui::End();
		}

		// Statistics Window
		if (m_ShowStats)
		{
			ImGui::Begin("Statistics", &m_ShowStats);

			ImGui::Text("Application Statistics");
			ImGui::Separator();

			auto& io = ImGui::GetIO();
			ImGui::Text("Frame Time: %.2f ms", m_LastFrameTime);
			ImGui::Text("FPS: %.1f", io.Framerate);
			ImGui::Text("Mouse Position: (%.0f, %.0f)", io.MousePos.x, io.MousePos.y);
			ImGui::Text("Mouse Clicks: %d", m_Clicks);

			ImGui::Separator();
			ImGui::Text("Memory");
			ImGui::BulletText("Vertices: N/A");
			ImGui::BulletText("Indices: N/A");
			ImGui::BulletText("Draw Calls: N/A");

			ImGui::Separator();
			ImGui::Text("Renderer");
			ImGui::BulletText("Backend: OpenGL");
			ImGui::BulletText("Version: 4.5+");

			ImGui::End();
		}

		// Profiler Panel
		if (m_ShowProfiler && m_ProfilerPanel)
		{
			m_ProfilerPanel->SetEnabled(m_ShowProfiler);
			m_ProfilerPanel->OnImGuiRender();
		}

		// Shader Editor Panel
		if (m_ShowShaderEditor && m_ShaderEditor)
		{
			m_ShaderEditor->SetEnabled(m_ShowShaderEditor);
			m_ShaderEditor->OnImGuiRender();
		}
		
		// Stats Panel (Renderer Statistics)
		if (m_ShowStatsPanel && m_StatsPanel)
		{
			m_StatsPanel->SetEnabled(m_ShowStatsPanel);
			m_StatsPanel->OnImGuiRender();
		}
		
		// Material Editor Panel
		if (m_ShowMaterialEditor && m_MaterialEditor)
		{
			m_MaterialEditor->SetEnabled(m_ShowMaterialEditor);
			m_MaterialEditor->OnImGuiRender();
		}
		
		// Model Viewer Panel
		if (m_ShowModelPanel && m_ModelPanel)
		{
			m_ModelPanel->SetEnabled(m_ShowModelPanel);
			m_ModelPanel->OnImGuiRender();
		}

		// ImGui Demo Window
		if (m_ShowDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDemoWindow);
	}

	void ImLayer::UpdateProfilerMetrics()
	{
		PROFILE_FUNC();

		if (!m_ProfilerPanel)
		return;

		// Update profiler with current metrics
		::Editor::PerformanceMetrics metrics;

		auto& io = ImGui::GetIO();
		metrics.FrameTime = m_LastFrameTime;
		metrics.FPS = io.Framerate;

		// TODO: Get actual memory stats from allocator
		metrics.AllocatedMemory = 0;
		metrics.UsedMemory = 0;
		metrics.FreeMemory = 0;

		// TODO: Get actual rendering stats from renderer
		metrics.DrawCalls = 0;
		metrics.Triangles = 0;
		metrics.Vertices = 0;

		m_ProfilerPanel->UpdateMetrics(metrics);
		
		// Update renderer stats panel
		if (m_StatsPanel)
		{
			::Editor::RendererStats stats;
			stats.FrameTime = m_LastFrameTime;
			stats.FPS = io.Framerate;
			
			// TODO: Get actual renderer statistics
			stats.DrawCalls = 0;
			stats.TriangleCount = 0;
			stats.VertexCount = 0;
			stats.IndexCount = 0;
			stats.TextureMemoryUsed = 0;
			stats.TextureMemoryAllocated = 0;
			stats.TextureCount = 0;
			stats.BufferMemoryUsed = 0;
			stats.VertexBufferCount = 0;
			stats.IndexBufferCount = 0;
			stats.UniformBufferCount = 0;
			stats.RenderPasses = 0;
			stats.ShaderSwitches = 0;
			
			m_StatsPanel->UpdateStats(stats);
		}
	}

	bool ImLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.IsRepeat())
			return false;

		switch (e.GetKeyCode())
		{
			case GLFW_KEY_F1:
				m_ShowDemoWindow = !m_ShowDemoWindow;
			return true;

			case GLFW_KEY_F2:
				m_ShowOverlay = !m_ShowOverlay;
			return true;

			case GLFW_KEY_F3:
				m_ShowProfiler = !m_ShowProfiler;
			return true;
						
			case GLFW_KEY_F4:
				m_ShowShaderEditor = !m_ShowShaderEditor;
			return true;
			
			case GLFW_KEY_F5:
				m_ShowStatsPanel = !m_ShowStatsPanel;
			return true;
			
			case GLFW_KEY_F6:
				m_ShowMaterialEditor = !m_ShowMaterialEditor;
			return true;
			
			case GLFW_KEY_F7:
				m_ShowModelPanel = !m_ShowModelPanel;
			return true;
		}

		return false;
	}

	bool ImLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		(void)e;
		m_Clicks++;
		return false;
	}

	void ImLayer::OnOverlayRender()
	{
		PROFILE_FUNC();

		const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 workPos = viewport->WorkPos;
		ImVec2 workSize = viewport->WorkSize;

		ImGui::SetNextWindowPos(ImVec2(workPos.x + workSize.x - 10, workPos.y + 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.35f);

		if (ImGui::Begin("Performance Overlay", &m_ShowOverlay, flags))
		{
			auto& io = ImGui::GetIO();
			ImGui::Text("Performance");
			ImGui::Separator();
			ImGui::Text("%.1f FPS (%.2f ms)", io.Framerate, m_LastFrameTime);
			ImGui::Separator();
			ImGui::TextDisabled("F1: Demo | F2: Overlay");
			ImGui::TextDisabled("F3: Profiler | F4: Shader");
			ImGui::TextDisabled("F5: Stats | F6: Material");
			ImGui::TextDisabled("F7: Model Viewer");
		}
		ImGui::End();
	}

	void ImLayer::ApplyCustomStyle()
	{
		PROFILE_FUNC();

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Modern dark theme with accent colors
		colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_ChildBg]                = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_PopupBg]                = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
		colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
		colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
		colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		// Style adjustments
		style.WindowPadding     = ImVec2(8, 8);
		style.FramePadding      = ImVec2(5, 2);
		style.CellPadding       = ImVec2(6, 6);
		style.ItemSpacing       = ImVec2(6, 6);
		style.ItemInnerSpacing  = ImVec2(6, 6);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing     = 25;
		style.ScrollbarSize     = 15;
		style.GrabMinSize       = 10;
		style.WindowBorderSize  = 1;
		style.ChildBorderSize   = 1;
		style.PopupBorderSize   = 1;
		style.FrameBorderSize   = 1;
		style.TabBorderSize     = 1;
		style.WindowRounding    = 7;
		style.ChildRounding     = 4;
		style.FrameRounding     = 3;
		style.PopupRounding     = 4;
		style.ScrollbarRounding = 9;
		style.GrabRounding      = 3;
		style.LogSliderDeadzone = 4;
		style.TabRounding       = 4;
	}
}
