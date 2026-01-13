#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Core/Renderer/Buffer.h"
#include "Core/Renderer/UniformBuffer.h"
#include "Core/Renderer/Material.h"

namespace Core::Renderer
{
    struct BatchStats
    {
        uint32_t DrawCalls = 0;
        uint32_t QuadCount = 0;
        uint32_t Flushes   = 0;

        void Reset()
        {
            DrawCalls = 0;
            QuadCount = 0;
            Flushes   = 0;
        }
    };

    class BatchRenderer2D
    {
    public:
        enum class Mode : uint8_t
        {
            CPUExpanded = 0, // classic: 4 verts per quad
            Instanced         // 1 instance per quad
        };

        BatchRenderer2D() = default;
        ~BatchRenderer2D();

        BatchRenderer2D(const BatchRenderer2D&) = delete;
        BatchRenderer2D& operator=(const BatchRenderer2D&) = delete;

        void Init(Material* material = nullptr);
        void Shutdown();

        void SetMode(Mode mode) { m_Mode = mode; }
        Mode GetMode() const { return m_Mode; }

        // Sorting = record sprites, sort by texture at EndScene (more batching, a bit more CPU)
        void SetSortByTexture(bool enabled) { m_SortByTexture = enabled; }
        bool GetSortByTexture() const { return m_SortByTexture; }

        void BeginScene(const glm::mat4& viewProjection);
        void EndScene();

        // Immediate draw (or queued if sorting enabled)
        void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        void DrawQuad(const glm::mat4& transform, GLuint textureID, const glm::vec4& color = glm::vec4(1.0f), float tiling = 1.0f);
        void DrawQuad(const glm::mat4& transform, GLuint textureID,
                      const glm::vec2& uvMin, const glm::vec2& uvMax,
                      const glm::vec4& color = glm::vec4(1.0f), float tiling = 1.0f);

        // Manual flush (usually you just EndScene)
        void Flush();

        const BatchStats& GetStats() const { return m_Stats; }
        void ResetStats() { m_Stats.Reset(); }

        GLuint GetWhiteTextureID() const { return m_WhiteTexture; }

    private:
        // ---- CPU expanded path ----
        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            float TexIndex;
            float Tiling;
        };

        // ---- Instanced path ----
        struct QuadInstance
        {
            glm::mat4 Transform;
            glm::vec4 Color;
            glm::vec4 UVMinMax;   // (u0,v0,u1,v1)
            glm::vec2 Tex;        // (texIndex, tiling)
            glm::vec2 _Pad;       // align 16
        };

        struct SpriteCmd
        {
            glm::mat4 Transform;
            glm::vec4 Color;
            GLuint TextureID;
            glm::vec2 UVMin;
            glm::vec2 UVMax;
            float Tiling;
        };

    private:
        void StartBatch();
        void NextBatch();

        // Adds quad to current batch (no sorting)
        void EmitQuadCPU(const glm::mat4& transform, GLuint texID, const glm::vec2& uvMin, const glm::vec2& uvMax,
                         const glm::vec4& color, float tiling);
        void EmitQuadInstanced(const glm::mat4& transform, GLuint texID, const glm::vec2& uvMin, const glm::vec2& uvMax,
                               const glm::vec4& color, float tiling);

        uint32_t AcquireTextureSlot(GLuint texID); // returns slot index (float stored)

        void EnsureMaterialAndUBO();
        void CreateWhiteTexture();
        void DestroyWhiteTexture();

        void SetupCPUPath();
        void SetupInstancedPath();

        void UploadAndDrawCPU();
        void UploadAndDrawInstanced();

        // Sorting playback
        void FlushSorted();

    private:
        // Limits
        static constexpr uint32_t MaxQuads    = 20000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices  = MaxQuads * 6;
        static constexpr uint32_t MaxInstances= MaxQuads;

        static constexpr uint32_t HardMaxTextureSlots = 32; // clamped to GL_MAX_TEXTURE_IMAGE_UNITS

    private:
        Mode m_Mode = Mode::CPUExpanded;
        bool m_SortByTexture = false;

        BatchStats m_Stats;

        // Shader/material
        Material* m_MaterialExternal = nullptr;
        Material  m_MaterialOwned;         // used if external == nullptr
        Material* m_Material = nullptr;

        // Per-frame UBO (binding 0). Expects "FrameData" block with u_ViewProjection
        UniformBuffer m_FrameUBO;
        std::string m_FrameBlockName = "FrameData";
        std::string m_ViewProjName   = "u_ViewProjection";

        // White texture (slot 0)
        GLuint m_WhiteTexture = 0;

        // Texture slots for current batch
        uint32_t m_MaxTextureSlots = 16;
        std::vector<GLuint> m_TextureSlots; // [0] = white
        std::unordered_map<GLuint, uint32_t> m_TextureSlotLUT;

        // ---- CPU Expanded ----
        GLuint m_VAO_CPU = 0;
        VertexBuffer m_VB_CPU;
        IndexBuffer  m_IB_CPU;

        std::vector<uint8_t> m_CPUVertexStorage;
        QuadVertex* m_CPUBase = nullptr;
        QuadVertex* m_CPUHead = nullptr;
        uint32_t m_CPUIndexCount = 0;

        // ---- Instanced ----
        GLuint m_VAO_Inst = 0;
        VertexBuffer m_VB_QuadBase;    // 4 verts
        IndexBuffer  m_IB_QuadBase;    // 6 indices
        VertexBuffer m_VB_Instance;    // instances

        std::vector<uint8_t> m_InstanceStorage;
        QuadInstance* m_InstBase = nullptr;
        QuadInstance* m_InstHead = nullptr;
        uint32_t m_InstanceCount = 0;

        // Sorting queue
        std::vector<SpriteCmd> m_SortedCmds;

        // Tiny state cache
        GLuint m_LastBoundVAO = 0;
        GLuint m_LastBoundProgram = 0;
    };
}
