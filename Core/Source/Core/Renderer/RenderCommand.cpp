#include "RenderCommand.h"

#include <cassert>
#include <cstring>

#include <glad/glad.h>

#include "Core/Renderer/UniformBuffer.h"
#include "Core/Renderer/Mesh.h"
#include "Core/Renderer/Material.h"

namespace Core::Renderer
{
    static constexpr uint32_t kHeaderAlign = 16;

    uint32_t RenderCommandBuffer::AlignUp(uint32_t v, uint32_t a)
    {
        return (v + (a - 1)) & ~(a - 1);
    }

    RenderCommandBuffer::RenderCommandBuffer(uint32_t reserveBytes)
    {
        m_Buffer.reserve(reserveBytes);
    }

    RenderCommandBuffer::RenderCommandBuffer(RenderCommandBuffer&& other) noexcept
    {
        *this = std::move(other);
    }

    RenderCommandBuffer& RenderCommandBuffer::operator=(RenderCommandBuffer&& other) noexcept
    {
        if (this == &other) return *this;
        m_Buffer = std::move(other.m_Buffer);
        m_WriteOffset = other.m_WriteOffset;
        other.m_WriteOffset = 0;
        return *this;
    }

    void RenderCommandBuffer::Clear()
    {
        m_Buffer.clear();
        m_WriteOffset = 0;
    }

    void RenderCommandBuffer::EnsureCapacity(uint32_t bytesNeeded)
    {
        if (m_Buffer.size() < m_WriteOffset + bytesNeeded)
            m_Buffer.resize(m_WriteOffset + bytesNeeded);
    }

    template<typename T, typename... Args>
    T* RenderCommandBuffer::Push(ExecuteFn fn, Args&&... args)
    {
        // Layout:
        // [padding]->Header->[padding]->Command
        uint32_t headerOffset = AlignUp(m_WriteOffset, kHeaderAlign);
        uint32_t cmdOffset = AlignUp(headerOffset + (uint32_t)sizeof(CommandHeader), (uint32_t)alignof(T));
        uint32_t endOffset = cmdOffset + (uint32_t)sizeof(T);
        uint32_t stride = endOffset - headerOffset;

        EnsureCapacity(stride + (headerOffset - m_WriteOffset));
        // If we aligned forward, bump the write cursor to header
        m_WriteOffset = headerOffset;

        auto* header = reinterpret_cast<CommandHeader*>(m_Buffer.data() + headerOffset);
        header->Execute = fn;
        header->CommandOffset = (cmdOffset - headerOffset);
        header->CommandSize = (uint32_t)sizeof(T);
        header->Stride = stride;

        void* cmdMem = (void*)(m_Buffer.data() + cmdOffset);
        T* cmd = new (cmdMem) T(std::forward<Args>(args)...);

        m_WriteOffset = headerOffset + stride;
        return cmd;
    }

    // ------------------ Commands ------------------

    struct ClearCmd
    {
        glm::vec4 Color;
        GLbitfield Mask;
        static void Exec(void* p, RenderCommandContext&)
        {
            auto& c = *reinterpret_cast<ClearCmd*>(p);
            glClearColor(c.Color.r, c.Color.g, c.Color.b, c.Color.a);
            glClear(c.Mask);
        }
    };

    void RenderCommandBuffer::CmdClear(const glm::vec4& color, bool colorBuffer, bool depthBuffer, bool stencilBuffer)
    {
        GLbitfield mask = 0;
        if (colorBuffer)   mask |= GL_COLOR_BUFFER_BIT;
        if (depthBuffer)   mask |= GL_DEPTH_BUFFER_BIT;
        if (stencilBuffer) mask |= GL_STENCIL_BUFFER_BIT;

        Push<ClearCmd>(&ClearCmd::Exec, ClearCmd{ color, mask });
    }

    struct ViewportCmd
    {
        int X, Y, W, H;
        static void Exec(void* p, RenderCommandContext&)
        {
            auto& c = *reinterpret_cast<ViewportCmd*>(p);
            glViewport(c.X, c.Y, c.W, c.H);
        }
    };

    void RenderCommandBuffer::CmdSetViewport(int x, int y, int w, int h)
    {
        Push<ViewportCmd>(&ViewportCmd::Exec, ViewportCmd{ x, y, w, h });
    }

    struct BindMaterialCmd
    {
        const Material* Mat;
        static void Exec(void* p, RenderCommandContext& ctx)
        {
            auto& c = *reinterpret_cast<BindMaterialCmd*>(p);
            ctx.BoundMaterial = c.Mat;
            ctx.BoundMaterialInstance = nullptr;
            if (c.Mat) c.Mat->Bind();
        }
    };

    void RenderCommandBuffer::CmdBindMaterial(const Material* material)
    {
        Push<BindMaterialCmd>(&BindMaterialCmd::Exec, BindMaterialCmd{ material });
    }

    struct BindMaterialInstanceCmd
    {
        const MaterialInstance* MatInst;
        static void Exec(void* p, RenderCommandContext& ctx)
        {
            auto& c = *reinterpret_cast<BindMaterialInstanceCmd*>(p);
            ctx.BoundMaterialInstance = c.MatInst;
            ctx.BoundMaterial = nullptr;
            if (c.MatInst) c.MatInst->Bind();
        }
    };

    void RenderCommandBuffer::CmdBindMaterialInstance(const MaterialInstance* materialInstance)
    {
        Push<BindMaterialInstanceCmd>(&BindMaterialInstanceCmd::Exec, BindMaterialInstanceCmd{ materialInstance });
    }

    struct SetPerFrameCmd
    {
        glm::mat4 ViewProj;
        float Time;
        glm::vec2 Resolution;

        static void Exec(void* p, RenderCommandContext& ctx)
        {
            auto& c = *reinterpret_cast<SetPerFrameCmd*>(p);
            if (!ctx.PerFrameUBO) return;

            if (ctx.PerFrameUBO->Has(ctx.PerFrame_ViewProj))
                ctx.PerFrameUBO->SetMat4(ctx.PerFrame_ViewProj, &c.ViewProj[0][0], false);

            if (ctx.PerFrameUBO->Has(ctx.PerFrame_Time))
                ctx.PerFrameUBO->SetFloat(ctx.PerFrame_Time, c.Time, false);

            if (ctx.PerFrameUBO->Has(ctx.PerFrame_Resolution))
            {
                float r[2] = { c.Resolution.x, c.Resolution.y };
                ctx.PerFrameUBO->SetVec2(ctx.PerFrame_Resolution, r, false);
            }

            ctx.PerFrameUBO->Upload();
            ctx.PerFrameUBO->BindBase();
        }
    };

    void RenderCommandBuffer::CmdSetPerFrame(const glm::mat4& viewProj, float timeSeconds, const glm::vec2& resolution)
    {
        Push<SetPerFrameCmd>(&SetPerFrameCmd::Exec, SetPerFrameCmd{ viewProj, timeSeconds, resolution });
    }

    struct SetModelCmd
    {
        glm::mat4 Model;
        static void Exec(void* p, RenderCommandContext& ctx)
        {
            auto& c = *reinterpret_cast<SetModelCmd*>(p);
            if (!ctx.PerObjectUBO) return;

            if (ctx.PerObjectUBO->Has(ctx.PerObject_Model))
            {
                ctx.PerObjectUBO->SetMat4(ctx.PerObject_Model, &c.Model[0][0], false);
                ctx.PerObjectUBO->Upload();
                ctx.PerObjectUBO->BindBase();
            }
        }
    };

    void RenderCommandBuffer::CmdSetModelMatrix(const glm::mat4& model)
    {
        Push<SetModelCmd>(&SetModelCmd::Exec, SetModelCmd{ model });
    }

    struct DrawMeshCmd
    {
        const Mesh* M;
        static void Exec(void* p, RenderCommandContext& ctx)
        {
            auto& c = *reinterpret_cast<DrawMeshCmd*>(p);
            if (!c.M) return;

            // If user didn’t explicitly bind material, we can still bind whatever state is set.
            if (ctx.BoundMaterialInstance) ctx.BoundMaterialInstance->Bind();
            else if (ctx.BoundMaterial)    ctx.BoundMaterial->Bind();

            c.M->Draw();
        }
    };

    void RenderCommandBuffer::CmdDrawMesh(const Mesh* mesh)
    {
        Push<DrawMeshCmd>(&DrawMeshCmd::Exec, DrawMeshCmd{ mesh });
    }

    void RenderCommandBuffer::Execute(RenderCommandContext& ctx) const
    {
        uint32_t offset = 0;
        while (offset < m_WriteOffset)
        {
            offset = AlignUp(offset, kHeaderAlign);
            if (offset >= m_WriteOffset) break;

            const auto* header = reinterpret_cast<const CommandHeader*>(m_Buffer.data() + offset);
            assert(header->Execute && "Render command missing execute fn!");

            void* cmdPtr = (void*)(m_Buffer.data() + offset + header->CommandOffset);
            header->Execute(cmdPtr, ctx);

            offset += header->Stride;
        }
    }
}
