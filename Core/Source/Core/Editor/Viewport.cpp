#include "Viewport.h"

#include <imgui/imgui.h>

// Your engine framebuffer
#include "Core/Renderer/Framebuffer.h"

namespace Core::Editor
{
    Viewport::Viewport(std::string name)
        : m_Name(std::move(name))
    {
    }

    void Viewport::SetFramebuffer(const std::shared_ptr<Core::Renderer::Framebuffer>& framebuffer,
                                  uint32_t colorAttachmentIndex)
    {
        m_Framebuffer = framebuffer;
        m_ColorAttachmentIndex = colorAttachmentIndex;
    }

    void Viewport::SetOnResizeCallback(std::function<void(uint32_t, uint32_t)> cb)
    {
        m_OnResize = std::move(cb);
    }

    void Viewport::OnImGuiRender()
    {
        m_ResizedThisFrame = false;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin(m_Name.c_str());

        m_Focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        m_Hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

        // Available draw region inside the window
        ImVec2 avail = ImGui::GetContentRegionAvail();
        glm::vec2 newSize = { avail.x, avail.y };

        // Bounds in screen space (top-left of image)
        ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
        m_Bounds[0] = { cursorScreen.x, cursorScreen.y };
        m_Bounds[1] = { cursorScreen.x + newSize.x, cursorScreen.y + newSize.y };

        // Resize framebuffer if needed
        if (m_Framebuffer && (newSize.x > 0.0f && newSize.y > 0.0f))
        {
            if (newSize != m_Size)
            {
                m_Size = newSize;

                // Round to integer pixels
                uint32_t w = (uint32_t)(m_Size.x);
                uint32_t h = (uint32_t)(m_Size.y);
                HandleResize(w, h);

                m_ResizedThisFrame = true;
            }
        }
        else
        {
            m_Size = newSize;
        }

        // Draw the framebuffer color attachment (OpenGL texture id -> ImTextureID)
        if (m_Framebuffer && m_Size.x > 0.0f && m_Size.y > 0.0f)
        {
            uint32_t texID = m_Framebuffer->GetColorAttachmentID(m_ColorAttachmentIndex);

            // NOTE: ImGui expects an opaque "texture handle" (ImTextureID).
            // For OpenGL backends it's typically the GLuint texture id cast to pointer-sized storage. :contentReference[oaicite:1]{index=1}
            ImTextureID imguiTex = (ImTextureID)(intptr_t)texID;

            // Flip UV vertically because OpenGL textures are bottom-left origin.
            ImGui::Image(imguiTex, ImVec2(m_Size.x, m_Size.y), ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::Dummy(ImVec2(avail.x, avail.y));
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void Viewport::HandleResize(uint32_t newW, uint32_t newH)
    {
        if (!m_Framebuffer)
            return;

        // Resize the framebuffer itself
        m_Framebuffer->Resize(newW, newH);

        // Let the editor/pipeline resize other stuff (render passes, camera aspect, etc.)
        if (m_OnResize)
            m_OnResize(newW, newH);
    }

    bool Viewport::GetMousePosInViewport(glm::vec2& outPos) const
    {
        ImVec2 mp = ImGui::GetMousePos();

        const float mx = mp.x - m_Bounds[0].x;
        const float my = mp.y - m_Bounds[0].y;

        if (mx < 0.0f || my < 0.0f || mx > m_Size.x || my > m_Size.y)
            return false;

        outPos = { mx, my };
        return true;
    }

    bool Viewport::GetMouseUV(glm::vec2& outUV) const
    {
        glm::vec2 p;
        if (!GetMousePosInViewport(p))
            return false;

        if (m_Size.x <= 0.0f || m_Size.y <= 0.0f)
            return false;

        outUV = { p.x / m_Size.x, p.y / m_Size.y }; // (0,0) top-left
        return true;
    }
}
