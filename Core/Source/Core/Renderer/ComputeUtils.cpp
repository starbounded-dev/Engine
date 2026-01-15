#include "ComputeUtils.h"

namespace Core::Renderer
{
    // SSBO implementation
    SSBO::~SSBO()
    {
        Destroy();
    }

    SSBO::SSBO(SSBO&& other) noexcept
        : m_Handle(other.m_Handle)
        , m_Size(other.m_Size)
    {
        other.m_Handle = 0;
        other.m_Size = 0;
    }

    SSBO& SSBO::operator=(SSBO&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_Handle = other.m_Handle;
            m_Size = other.m_Size;

            other.m_Handle = 0;
            other.m_Size = 0;
        }
        return *this;
    }

    void SSBO::Create(uint32_t size, const void* data, GLenum usage)
    {
        Destroy();

        m_Size = size;

        glGenBuffers(1, &m_Handle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void SSBO::Destroy()
    {
        if (m_Handle)
        {
            glDeleteBuffers(1, &m_Handle);
            m_Handle = 0;
        }
        m_Size = 0;
    }

    void SSBO::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        if (!m_Handle || offset + size > m_Size)
            return;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void SSBO::BindBase(uint32_t bindingPoint) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_Handle);
    }

    void* SSBO::Map(GLenum access)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Handle);
        return glMapBuffer(GL_SHADER_STORAGE_BUFFER, access);
    }

    void SSBO::Unmap()
    {
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    // ComputeShader implementation
    ComputeShader::ComputeShader(uint32_t program)
        : m_Program(program)
    {
    }

    void ComputeShader::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        if (!m_Program)
            return;

        glUseProgram(m_Program);
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    }

    void ComputeShader::DispatchIndirect(GLuint indirectBuffer, uint32_t offset)
    {
        if (!m_Program)
            return;

        glUseProgram(m_Program);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer);
        glDispatchComputeIndirect(offset);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    }

    void ComputeShader::MemoryBarrier(GLbitfield barriers)
    {
        glMemoryBarrier(barriers);
    }

    void ComputeShader::GetWorkGroupSize(uint32_t& x, uint32_t& y, uint32_t& z) const
    {
        if (!m_Program)
        {
            x = y = z = 1;
            return;
        }

        int workGroupSize[3];
        glGetProgramiv(m_Program, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
        x = workGroupSize[0];
        y = workGroupSize[1];
        z = workGroupSize[2];
    }

    // GPUParticleSystem implementation (placeholder)
    void GPUParticleSystem::Init(uint32_t maxParticles)
    {
        m_MaxParticles = maxParticles;
        m_AliveCount = 0;

        // Create particle buffer
        m_ParticleBuffer.Create(maxParticles * sizeof(ParticleData), nullptr, GL_DYNAMIC_DRAW);

        // Create indirect draw buffer
        IndirectDrawCommand cmd = { 0, 0, 0, 0 };
        m_IndirectBuffer.Create(sizeof(IndirectDrawCommand), &cmd, GL_DYNAMIC_DRAW);

        // Note: Load compute shader and render shader in a real implementation
        // m_UpdateShader.SetProgram(CreateComputeShader("particles_update.comp"));
        // m_RenderProgram = CreateGraphicsShader("particles.vert", "particles.frag");

        // Create VAO for rendering
        glGenVertexArrays(1, &m_VAO);
    }

    void GPUParticleSystem::Shutdown()
    {
        m_ParticleBuffer.Destroy();
        m_IndirectBuffer.Destroy();

        if (m_VAO)
        {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
    }

    void GPUParticleSystem::Update(float deltaTime)
    {
        // Bind particle buffer to SSBO binding point
        m_ParticleBuffer.BindBase(0);

        // Dispatch compute shader (one thread per particle)
        uint32_t numGroups = (m_MaxParticles + 255) / 256; // 256 threads per group
        m_UpdateShader.Dispatch(numGroups, 1, 1);

        // Memory barrier to ensure compute writes are visible
        ComputeShader::MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void GPUParticleSystem::Render()
    {
        if (!m_RenderProgram)
            return;

        glUseProgram(m_RenderProgram);
        glBindVertexArray(m_VAO);

        // Bind particle buffer for instanced rendering
        m_ParticleBuffer.BindBase(0);

        // Draw using indirect buffer (GPU determines instance count)
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_IndirectBuffer.GetHandle());
        glDrawArraysIndirect(GL_POINTS, nullptr);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

        glBindVertexArray(0);
    }

    void GPUParticleSystem::Emit(uint32_t count, const ParticleData& template_particle)
    {
        // Placeholder: Add new particles to the buffer
        // This would typically be done via compute shader or CPU update
        m_AliveCount = std::min(m_AliveCount + count, m_MaxParticles);
    }

    // GPUCulling implementation (placeholder)
    void GPUCulling::Init(uint32_t maxObjects)
    {
        m_MaxObjects = maxObjects;

        m_BoundsBuffer.Create(maxObjects * sizeof(ObjectBounds), nullptr, GL_DYNAMIC_DRAW);
        m_ResultsBuffer.Create(maxObjects * sizeof(CullResult), nullptr, GL_DYNAMIC_DRAW);

        // Note: Load culling compute shader
        // m_CullShader.SetProgram(CreateComputeShader("frustum_cull.comp"));
    }

    void GPUCulling::Shutdown()
    {
        m_BoundsBuffer.Destroy();
        m_ResultsBuffer.Destroy();
    }

    void GPUCulling::CullObjects(const float* viewProjectionMatrix,
        const std::vector<ObjectBounds>& bounds,
        std::vector<uint32_t>& visibleIndices)
    {
        visibleIndices.clear();

        if (bounds.empty() || !m_CullShader.GetProgram())
            return;

        // Upload bounds to GPU
        m_BoundsBuffer.SetData(bounds.data(), static_cast<uint32_t>(bounds.size() * sizeof(ObjectBounds)));

        // Bind buffers
        m_BoundsBuffer.BindBase(0);
        m_ResultsBuffer.BindBase(1);

        // Set view-projection matrix uniform
        // glUniformMatrix4fv(...);

        // Dispatch compute shader
        uint32_t numGroups = (static_cast<uint32_t>(bounds.size()) + 255) / 256;
        m_CullShader.Dispatch(numGroups, 1, 1);

        // Memory barrier
        ComputeShader::MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Read back results
        CullResult* results = static_cast<CullResult*>(m_ResultsBuffer.Map(GL_READ_ONLY));
        if (results)
        {
            for (size_t i = 0; i < bounds.size(); ++i)
            {
                if (results[i].Visible)
                {
                    visibleIndices.push_back(static_cast<uint32_t>(i));
                }
            }
            m_ResultsBuffer.Unmap();
        }
    }

    // IndirectDrawBuffer implementation
    IndirectDrawBuffer::~IndirectDrawBuffer()
    {
        Destroy();
    }

    void IndirectDrawBuffer::Create(uint32_t maxCommands, bool indexed)
    {
        Destroy();

        m_MaxCommands = maxCommands;
        m_Indexed = indexed;

        uint32_t commandSize = indexed ? sizeof(IndirectDrawElementsCommand) : sizeof(IndirectDrawCommand);
        uint32_t bufferSize = maxCommands * commandSize;

        glGenBuffers(1, &m_Handle);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    void IndirectDrawBuffer::Destroy()
    {
        if (m_Handle)
        {
            glDeleteBuffers(1, &m_Handle);
            m_Handle = 0;
        }
        m_MaxCommands = 0;
    }

    void IndirectDrawBuffer::SetCommands(const void* commands, uint32_t count)
    {
        if (!m_Handle || count > m_MaxCommands)
            return;

        uint32_t commandSize = m_Indexed ? sizeof(IndirectDrawElementsCommand) : sizeof(IndirectDrawCommand);
        uint32_t dataSize = count * commandSize;

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
        glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0, dataSize, commands);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    void IndirectDrawBuffer::Bind() const
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);
    }

    void IndirectDrawBuffer::Draw(uint32_t commandCount, uint32_t offset) const
    {
        if (!m_Handle)
            return;

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_Handle);

        uint32_t commandSize = m_Indexed ? sizeof(IndirectDrawElementsCommand) : sizeof(IndirectDrawCommand);
        void* cmdOffset = reinterpret_cast<void*>(static_cast<uintptr_t>(offset * commandSize));

        if (m_Indexed)
        {
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, cmdOffset, commandCount, 0);
        }
        else
        {
            glMultiDrawArraysIndirect(GL_TRIANGLES, cmdOffset, commandCount, 0);
        }

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }
}
