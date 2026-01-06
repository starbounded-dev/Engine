#include "ImLayer.h"

#include "Core/Application.h"
#include "Core/Renderer/Renderer.h"

#include "Core/Debug/Profiler.h"

#include <imgui.h>
#include <GLFW/glfw3.h> // for GLFW_KEY_F1/F2

#include "glad/glad.h"

namespace Core
{
	ImLayer::ImLayer()
		: Layer("ImLayer")
	{
	}

	void ImLayer::OnAttach()
	{
		// init your state here if needed
	}

	void ImLayer::OnDetach()
	{
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
	}

	void ImLayer::OnImGuiRender()
	{
		PROFILE_FUNC();

		static bool dockspaceOpen = true;
		static bool fullscreen = true;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking;

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

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpace", &dockspaceOpen, windowFlags);

		ImGui::PopStyleVar(2);

		// DockSpace node
		ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
					Core::Application::Get().Stop();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("ImLayer", nullptr, nullptr, false); // just label
				ImGui::MenuItem("Demo", nullptr, &m_ShowDemoWindow);
				ImGui::MenuItem("Overlay", nullptr, &m_ShowOverlay);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End(); // DockSpace host

		// ---- Your dockable windows below ----

		if (m_ShowDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDemoWindow);

		ImGui::Begin("ImLayer");
		ImGui::Text("Hello from ImLayer");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("Mouse clicks: %d", m_Clicks);
		ImGui::End();

		if (m_ShowOverlay)
			OnOverlayRender(); // consider adding NoDocking flags inside overlay
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
		}

		return false;
	}

	bool ImLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		(void)e;
		m_Clicks++;
		return false; // don’t “consume” it unless you want to block other layers
	}

	void ImLayer::OnOverlayRender()
	{
		PROFILE_FUNC();

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

		if (ImGui::Begin("Overlay", &m_ShowOverlay, flags))
		{
			auto& io = ImGui::GetIO();
			ImGui::Text("Overlay");
			ImGui::Separator();
			ImGui::Text("FPS: %.1f (%.2f ms)", io.Framerate, (io.Framerate > 0.0f) ? (1000.0f / io.Framerate) : 0.0f);
			ImGui::Text("Clicks: %d", m_Clicks);
		}
		ImGui::End();
	}
}
