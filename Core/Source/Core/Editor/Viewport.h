#pragma once
#include <string>
#include <array>
#include <memory>
#include <functional>

#include <glm/glm.hpp>

namespace Core::Renderer { class Framebuffer; }

namespace Core::Editor
{
    class Viewport
    {
    public:
        explicit Viewport(std::string name = "Viewport");

        // Call every frame from your EditorLayer::OnImGuiRender()
        void OnImGuiRender();

        // Provide the framebuffer you want to display (usually lighting output)
        void SetFramebuffer(const std::shared_ptr<Core::Renderer::Framebuffer>& framebuffer,
                            uint32_t colorAttachmentIndex = 0);

        // Optional: if your render pipeline needs extra resize handling
        // (e.g. resizing render passes, camera aspect, etc.)
        void SetOnResizeCallback(std::function<void(uint32_t, uint32_t)> cb);

        const std::string& GetName() const { return m_Name; }

        bool IsFocused() const { return m_Focused; }
        bool IsHovered() const { return m_Hovered; }

        glm::vec2 GetSize() const { return m_Size; }
        std::array<glm::vec2, 2> GetBounds() const { return m_Bounds; } // screen-space min/max

        // Mouse position in viewport local pixels (0..size). Y is top->bottom.
        // Returns false if mouse is outside viewport.
        bool GetMousePosInViewport(glm::vec2& outPos) const;

        // UV (0..1) inside viewport. (0,0) top-left. Returns false if outside.
        bool GetMouseUV(glm::vec2& outUV) const;

        // True when panel size changed this frame
        bool WasResizedThisFrame() const { return m_ResizedThisFrame; }

    private:
        void HandleResize(uint32_t newW, uint32_t newH);

    private:
        std::string m_Name;

        std::shared_ptr<Core::Renderer::Framebuffer> m_Framebuffer;
        uint32_t m_ColorAttachmentIndex = 0;

        glm::vec2 m_Size{ 0.0f };
        std::array<glm::vec2, 2> m_Bounds{ glm::vec2(0.0f), glm::vec2(0.0f) };

        bool m_Focused = false;
        bool m_Hovered = false;
        bool m_ResizedThisFrame = false;

        std::function<void(uint32_t, uint32_t)> m_OnResize;
    };
}
