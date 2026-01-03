#include "ImLayer.h"

#include "Core/Application.h"
#include "Core/Renderer/Renderer.h"

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
		auto size = Core::Application::Get().GetFramebufferSize();

		Renderer::BeginFrame((int)size.x, (int)size.y);
	}

	void ImLayer::OnImGuiRender()
	{
		// 1) optional: imgui demo
		if (m_ShowDemoWindow)
			ImGui::ShowDemoWindow(&m_ShowDemoWindow);

		// 2) your window
		ImGui::Begin("ImLayer");
		ImGui::Text("Hello from ImLayer");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

		ImGui::Checkbox("Show Demo Window (F1)", &m_ShowDemoWindow);
		ImGui::Checkbox("Show Overlay (F2)", &m_ShowOverlay);

		ImGui::Separator();
		ImGui::Text("Mouse clicks: %d", m_Clicks);
		ImGui::End();

		// 3) overlay
		if (m_ShowOverlay)
			OnOverlayRender();
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
