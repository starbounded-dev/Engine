#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace Core::Renderer
{
    // Shader Storage Buffer Object (SSBO) for compute shaders
    class SSBO
    {
    public:
        SSBO() = default;
        ~SSBO();

        SSBO(const SSBO&) = delete;
        SSBO& operator=(const SSBO&) = delete;

        SSBO(SSBO&& other) noexcept;
        SSBO& operator=(SSBO&& other) noexcept;

        // Create SSBO with initial data
        void Create(uint32_t size, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW);
        void Destroy();

        // Update SSBO data
        void SetData(const void* data, uint32_t size, uint32_t offset = 0);

        // Bind to a binding point
        void BindBase(uint32_t bindingPoint) const;

        // Map/unmap for reading/writing
        void* Map(GLenum access = GL_READ_WRITE);
        void Unmap();

        GLuint GetHandle() const { return m_Handle; }
        uint32_t GetSize() const { return m_Size; }

    private:
        GLuint m_Handle = 0;
        uint32_t m_Size = 0;
    };

    // Compute shader dispatcher
    class ComputeShader
    {
    public:
        ComputeShader() = default;
        explicit ComputeShader(uint32_t program);

        void SetProgram(uint32_t program) { m_Program = program; }
        uint32_t GetProgram() const { return m_Program; }

        // Dispatch compute shader
        void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

        // Dispatch with indirect buffer
        void DispatchIndirect(GLuint indirectBuffer, uint32_t offset = 0);

        // Wait for compute to finish
        static void MemoryBarrier(GLbitfield barriers = GL_ALL_BARRIER_BITS);

        // Get work group size from shader
        void GetWorkGroupSize(uint32_t& x, uint32_t& y, uint32_t& z) const;

    private:
        uint32_t m_Program = 0;
    };

    // GPU particles system using compute shaders
    class GPUParticleSystem
    {
    public:
        struct ParticleData
        {
            float PositionX, PositionY, PositionZ, LifeTime;
            float VelocityX, VelocityY, VelocityZ, Age;
            float ColorR, ColorG, ColorB, ColorA;
            float SizeX, SizeY, Rotation, Reserved;
        };

        GPUParticleSystem() = default;

        void Init(uint32_t maxParticles);
        void Shutdown();

        // Update particles on GPU
        void Update(float deltaTime);

        // Render particles
        void Render();

        // Emit new particles
        void Emit(uint32_t count, const ParticleData& template_particle);

        uint32_t GetMaxParticles() const { return m_MaxParticles; }
        uint32_t GetAliveCount() const { return m_AliveCount; }

    private:
        uint32_t m_MaxParticles = 0;
        uint32_t m_AliveCount = 0;

        SSBO m_ParticleBuffer;
        SSBO m_IndirectBuffer; // For indirect rendering

        ComputeShader m_UpdateShader;
        uint32_t m_RenderProgram = 0;
        GLuint m_VAO = 0;
    };

    // GPU frustum culling
    class GPUCulling
    {
    public:
        struct ObjectBounds
        {
            float CenterX, CenterY, CenterZ, Radius;
        };

        struct CullResult
        {
            uint32_t Visible; // 1 if visible, 0 if culled
            uint32_t Reserved[3];
        };

        GPUCulling() = default;

        void Init(uint32_t maxObjects);
        void Shutdown();

        // Perform frustum culling on GPU
        void CullObjects(const float* viewProjectionMatrix,
            const std::vector<ObjectBounds>& bounds,
            std::vector<uint32_t>& visibleIndices);

    private:
        uint32_t m_MaxObjects = 0;

        SSBO m_BoundsBuffer;
        SSBO m_ResultsBuffer;

        ComputeShader m_CullShader;
    };

    // Indirect drawing support
    struct IndirectDrawCommand
    {
        uint32_t Count;         // vertex/index count
        uint32_t InstanceCount; // instance count
        uint32_t First;         // first vertex/index
        uint32_t BaseInstance;  // base instance
    };

    struct IndirectDrawElementsCommand
    {
        uint32_t Count;         // index count
        uint32_t InstanceCount; // instance count
        uint32_t FirstIndex;    // first index
        int32_t  BaseVertex;    // base vertex
        uint32_t BaseInstance;  // base instance
    };

    // Indirect draw buffer manager
    class IndirectDrawBuffer
    {
    public:
        IndirectDrawBuffer() = default;
        ~IndirectDrawBuffer();

        IndirectDrawBuffer(const IndirectDrawBuffer&) = delete;
        IndirectDrawBuffer& operator=(const IndirectDrawBuffer&) = delete;

        // Create buffer for draw commands
        void Create(uint32_t maxCommands, bool indexed = true);
        void Destroy();

        // Update commands
        void SetCommands(const void* commands, uint32_t count);

        // Bind for indirect drawing
        void Bind() const;

        // Execute indirect draw
        void Draw(uint32_t commandCount, uint32_t offset = 0) const;

        GLuint GetHandle() const { return m_Handle; }

    private:
        GLuint m_Handle = 0;
        uint32_t m_MaxCommands = 0;
        bool m_Indexed = true;
    };
}
