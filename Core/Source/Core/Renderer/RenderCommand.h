#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include <glm/glm.hpp>

namespace Core::Renderer
{
    class Material;
    class MaterialInstance;
    class Mesh;
    class UniformBuffer;

    struct RenderCommandContext
    {
        // Optional UBOs (if set, commands can update them)
        UniformBuffer* PerFrameUBO  = nullptr; // binding 0
        UniformBuffer* PerObjectUBO = nullptr; // binding 1

        // Names inside those UBOs (only used if UBO + name exists)
        std::string PerFrame_ViewProj = "u_ViewProjection";
        std::string PerFrame_Time     = "u_Time";
        std::string PerFrame_Resolution = "u_Resolution";
        std::string PerObject_Model   = "u_Model";

        // Stateful bindings
        const Material*         BoundMaterial = nullptr;
        const MaterialInstance* BoundMaterialInstance = nullptr;

        void ResetState()
        {
            BoundMaterial = nullptr;
            BoundMaterialInstance = nullptr;
        }
    };

    class RenderCommandBuffer
    {
    public:
        RenderCommandBuffer() = default;
        explicit RenderCommandBuffer(uint32_t reserveBytes);

        RenderCommandBuffer(const RenderCommandBuffer&) = delete;
        RenderCommandBuffer& operator=(const RenderCommandBuffer&) = delete;

        RenderCommandBuffer(RenderCommandBuffer&&) noexcept;
        RenderCommandBuffer& operator=(RenderCommandBuffer&&) noexcept;

        void Clear();
        bool Empty() const { return m_WriteOffset == 0; }

        // ---- High-level commands ----
        void CmdClear(const glm::vec4& color, bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = false);
        void CmdSetViewport(int x, int y, int w, int h);

        void CmdBindMaterial(const Material* material);
        void CmdBindMaterialInstance(const MaterialInstance* materialInstance);

        // Optional UBO updates (safe: will no-op if UBO missing or name not in layout)
        void CmdSetPerFrame(const glm::mat4& viewProj, float timeSeconds, const glm::vec2& resolution);
        void CmdSetModelMatrix(const glm::mat4& model);

        void CmdDrawMesh(const Mesh* mesh);

        // Execute recorded commands (call on render thread)
        void Execute(RenderCommandContext& ctx) const;

    private:
        using ExecuteFn = void(*)(void* cmd, RenderCommandContext& ctx);

        struct CommandHeader
        {
            ExecuteFn Execute = nullptr;
            uint32_t  CommandOffset = 0; // from start of header
            uint32_t  CommandSize = 0;
            uint32_t  Stride = 0;        // bytes to next header
        };

        static uint32_t AlignUp(uint32_t v, uint32_t a);

        template<typename T, typename... Args>
        T* Push(ExecuteFn fn, Args&&... args);

        void EnsureCapacity(uint32_t bytesNeeded);

    private:
        std::vector<uint8_t> m_Buffer;
        uint32_t m_WriteOffset = 0;
    };
}
