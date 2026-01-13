#include "BatchRenderer.h"
#include <algorithm>
#include <cstring>
#include <cassert>

namespace Core::Renderer
{
    static inline void BindVAO(GLuint& cache, GLuint vao)
    {
        if (cache != vao)
        {
            glBindVertexArray(vao);
            cache = vao;
        }
    }

    static inline void UseProgram(GLuint& cache, GLuint program)
    {
        if (cache != program)
        {
            glUseProgram(program);
            cache = program;
        }
    }

    BatchRenderer2D::~BatchRenderer2D()
    {
        Shutdown();
    }

    void BatchRenderer2D::Init(Material* material)
    {
        // Texture unit clamp
        GLint units = 16;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units);
        m_MaxTextureSlots = (uint32_t)std::min(units, (GLint)HardMaxTextureSlots);

        // Material: external or owned
        m_MaterialExternal = material;
        if (m_MaterialExternal)
            m_Material = m_MaterialExternal;
        else
        {
            // You need these shader files in your Resources/Shaders/
            // - Batch2D_CPU.vert.glsl / Batch2D_CPU.frag.glsl   (Mode::CPUExpanded)
            // - Batch2D_Inst.vert.glsl / Batch2D_Inst.frag.glsl (Mode::Instanced)
            // You can also just point both to the same frag if you want.
            m_MaterialOwned = Material(
                "Resources/Shaders/Batch2D_CPU.vert.glsl",
                "Resources/Shaders/Batch2D_CPU.frag.glsl"
            );
            m_Material = &m_MaterialOwned;
        }

        CreateWhiteTexture();

        // slot 0 = white
        m_TextureSlots.reserve(m_MaxTextureSlots);
        m_TextureSlots.clear();
        m_TextureSlotLUT.clear();
        m_TextureSlots.push_back(m_WhiteTexture);
        m_TextureSlotLUT[m_WhiteTexture] = 0;

        SetupCPUPath();
        SetupInstancedPath();

        EnsureMaterialAndUBO();
        StartBatch();
    }

    void BatchRenderer2D::Shutdown()
    {
        if (m_VAO_CPU) { glDeleteVertexArrays(1, &m_VAO_CPU); m_VAO_CPU = 0; }
        if (m_VAO_Inst){ glDeleteVertexArrays(1, &m_VAO_Inst); m_VAO_Inst = 0; }

        m_CPUVertexStorage.clear();
        m_CPUBase = m_CPUHead = nullptr;

        m_InstanceStorage.clear();
        m_InstBase = m_InstHead = nullptr;

        m_SortedCmds.clear();
        DestroyWhiteTexture();

        m_Material = nullptr;
        m_MaterialExternal = nullptr;
    }

    void BatchRenderer2D::CreateWhiteTexture()
    {
        if (m_WhiteTexture) return;

        uint32_t white = 0xFFFFFFFFu;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_WhiteTexture);
        glTextureStorage2D(m_WhiteTexture, 1, GL_RGBA8, 1, 1);
        glTextureSubImage2D(m_WhiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &white);

        glTextureParameteri(m_WhiteTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_WhiteTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_WhiteTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_WhiteTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void BatchRenderer2D::DestroyWhiteTexture()
    {
        if (m_WhiteTexture)
        {
            glDeleteTextures(1, &m_WhiteTexture);
            m_WhiteTexture = 0;
        }
    }

    void BatchRenderer2D::SetupCPUPath()
    {
        if (m_VAO_CPU) return;

        glCreateVertexArrays(1, &m_VAO_CPU);

        // dynamic VB storage
        m_VB_CPU = VertexBuffer((uint32_t)(MaxVertices * sizeof(QuadVertex)), BufferUsage::Dynamic);
        m_VB_CPU.SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color"    },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_Tiling"   },
        });

        // static IB indices
        std::vector<uint32_t> indices(MaxIndices);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }
        m_IB_CPU = IndexBuffer(indices.data(), (uint32_t)indices.size(), true, BufferUsage::Static);

        uint32_t attrib = 0;
        m_VB_CPU.BindToVertexArray(m_VAO_CPU, 0, attrib);
        m_IB_CPU.BindToVertexArray(m_VAO_CPU);

        // CPU staging
        m_CPUVertexStorage.resize(MaxVertices * sizeof(QuadVertex));
        m_CPUBase = reinterpret_cast<QuadVertex*>(m_CPUVertexStorage.data());
        m_CPUHead = m_CPUBase;
    }

    void BatchRenderer2D::SetupInstancedPath()
    {
        if (m_VAO_Inst) return;

        glCreateVertexArrays(1, &m_VAO_Inst);

        // Base quad vertices (pos+uv)
        struct BaseV { glm::vec3 Pos; glm::vec2 UV; };
        BaseV base[4] = {
            { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f} },
            { { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f} },
            { { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f} },
            { {-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f} },
        };

        uint32_t idx[6] = { 0,1,2, 2,3,0 };

        m_VB_QuadBase = VertexBuffer(base, sizeof(base), BufferUsage::Static);
        m_VB_QuadBase.SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" },
        });

        m_IB_QuadBase = IndexBuffer(idx, 6, true, BufferUsage::Static);

        // Instance buffer
        m_VB_Instance = VertexBuffer((uint32_t)(MaxInstances * sizeof(QuadInstance)), BufferUsage::Dynamic);

        // Bind base VB using your helper
        uint32_t attrib = 0;
        m_VB_QuadBase.BindToVertexArray(m_VAO_Inst, 0, attrib);
        m_IB_QuadBase.BindToVertexArray(m_VAO_Inst);

        // Manually describe instance attributes (binding = 1)
        glVertexArrayVertexBuffer(m_VAO_Inst, 1, m_VB_Instance.GetRendererID(), 0, sizeof(QuadInstance));

        // locations:
        // 2..5 = mat4 Transform
        // 6    = vec4 Color
        // 7    = vec4 UVMinMax
        // 8    = vec2 Tex (texIndex, tiling)
        for (int c = 0; c < 4; c++)
        {
            glEnableVertexArrayAttrib(m_VAO_Inst, 2 + c);
            glVertexArrayAttribFormat(m_VAO_Inst, 2 + c, 4, GL_FLOAT, GL_FALSE, (GLuint)(offsetof(QuadInstance, Transform) + sizeof(glm::vec4) * c));
            glVertexArrayAttribBinding(m_VAO_Inst, 2 + c, 1);
        }

        glEnableVertexArrayAttrib(m_VAO_Inst, 6);
        glVertexArrayAttribFormat(m_VAO_Inst, 6, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof(QuadInstance, Color));
        glVertexArrayAttribBinding(m_VAO_Inst, 6, 1);

        glEnableVertexArrayAttrib(m_VAO_Inst, 7);
        glVertexArrayAttribFormat(m_VAO_Inst, 7, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof(QuadInstance, UVMinMax));
        glVertexArrayAttribBinding(m_VAO_Inst, 7, 1);

        glEnableVertexArrayAttrib(m_VAO_Inst, 8);
        glVertexArrayAttribFormat(m_VAO_Inst, 8, 2, GL_FLOAT, GL_FALSE, (GLuint)offsetof(QuadInstance, Tex));
        glVertexArrayAttribBinding(m_VAO_Inst, 8, 1);

        // Divisor: per instance
        glVertexArrayBindingDivisor(m_VAO_Inst, 1, 1);

        // CPU staging
        m_InstanceStorage.resize(MaxInstances * sizeof(QuadInstance));
        m_InstBase = reinterpret_cast<QuadInstance*>(m_InstanceStorage.data());
        m_InstHead = m_InstBase;
    }

    void BatchRenderer2D::EnsureMaterialAndUBO()
    {
        assert(m_Material);

        // Make sure sampler array is mapped to [0..MaxTextureSlots-1]
        // Works even if your Material falls back to glUniform.
        for (uint32_t i = 0; i < m_MaxTextureSlots; i++)
        {
            // "u_Textures[0]" etc
            std::string name = "u_Textures[" + std::to_string(i) + "]";
            m_Material->SetInt(name, (int32_t)i);
        }

        // Reflect FrameData UBO from the material program (block should be in your shader)
        GLuint prog = m_Material->GetProgram();
        auto frameLayout = UniformBufferLayout::Reflect(prog, m_FrameBlockName);
        if (frameLayout.GetSize() > 0)
            m_FrameUBO = UniformBuffer(frameLayout, UBOBinding::PerFrame, true);
    }

    void BatchRenderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        // If sorting, we just record draws
        if (!m_SortByTexture)
            StartBatch();

        // Update per-frame UBO if present
        if (m_FrameUBO.GetRendererID() != 0)
        {
            // supports either "u_ViewProjection" or "FrameData.u_ViewProjection"
            const char* a = "u_ViewProjection";
            const std::string b = m_FrameBlockName + ".u_ViewProjection";

            if (m_FrameUBO.Has(a))
                m_FrameUBO.SetMat4(a, &viewProjection[0][0], false);
            else if (m_FrameUBO.Has(b))
                m_FrameUBO.SetMat4(b, &viewProjection[0][0], false);

            m_FrameUBO.Upload();
        }
    }

    void BatchRenderer2D::EndScene()
    {
        if (m_SortByTexture)
            FlushSorted();
        else
            Flush();
    }

    void BatchRenderer2D::StartBatch()
    {
        m_CPUIndexCount = 0;
        m_CPUHead = m_CPUBase;

        m_InstanceCount = 0;
        m_InstHead = m_InstBase;

        // reset texture slots each batch
        m_TextureSlots.clear();
        m_TextureSlotLUT.clear();
        m_TextureSlots.push_back(m_WhiteTexture);
        m_TextureSlotLUT[m_WhiteTexture] = 0;
    }

    void BatchRenderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    uint32_t BatchRenderer2D::AcquireTextureSlot(GLuint texID)
    {
        if (texID == 0) texID = m_WhiteTexture;

        auto it = m_TextureSlotLUT.find(texID);
        if (it != m_TextureSlotLUT.end())
            return it->second;

        // New texture for this batch
        if ((uint32_t)m_TextureSlots.size() >= m_MaxTextureSlots)
        {
            NextBatch();
            // after NextBatch() we have only white again
            it = m_TextureSlotLUT.find(texID);
            if (it != m_TextureSlotLUT.end())
                return it->second;
        }

        uint32_t slot = (uint32_t)m_TextureSlots.size();
        m_TextureSlots.push_back(texID);
        m_TextureSlotLUT[texID] = slot;
        return slot;
    }

    void BatchRenderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        DrawQuad(transform, m_WhiteTexture, {0,0}, {1,1}, color, 1.0f);
    }

    void BatchRenderer2D::DrawQuad(const glm::mat4& transform, GLuint textureID, const glm::vec4& color, float tiling)
    {
        DrawQuad(transform, textureID, {0,0}, {1,1}, color, tiling);
    }

    void BatchRenderer2D::DrawQuad(const glm::mat4& transform, GLuint textureID,
                                   const glm::vec2& uvMin, const glm::vec2& uvMax,
                                   const glm::vec4& color, float tiling)
    {
        if (m_SortByTexture)
        {
            m_SortedCmds.push_back({ transform, color, textureID ? textureID : m_WhiteTexture, uvMin, uvMax, tiling });
            return;
        }

        if (m_Mode == Mode::CPUExpanded)
            EmitQuadCPU(transform, textureID, uvMin, uvMax, color, tiling);
        else
            EmitQuadInstanced(transform, textureID, uvMin, uvMax, color, tiling);

        m_Stats.QuadCount++;
    }

    void BatchRenderer2D::EmitQuadCPU(const glm::mat4& transform, GLuint texID,
                                     const glm::vec2& uvMin, const glm::vec2& uvMax,
                                     const glm::vec4& color, float tiling)
    {
        if (m_CPUIndexCount + 6 > MaxIndices)
            NextBatch();

        const float texIndex = (float)AcquireTextureSlot(texID);

        static const glm::vec4 localPos[4] = {
            {-0.5f, -0.5f, 0.0f, 1.0f},
            { 0.5f, -0.5f, 0.0f, 1.0f},
            { 0.5f,  0.5f, 0.0f, 1.0f},
            {-0.5f,  0.5f, 0.0f, 1.0f},
        };

        glm::vec2 uvs[4] = {
            {uvMin.x, uvMin.y},
            {uvMax.x, uvMin.y},
            {uvMax.x, uvMax.y},
            {uvMin.x, uvMax.y},
        };

        for (int i = 0; i < 4; i++)
        {
            glm::vec4 world = transform * localPos[i];
            m_CPUHead->Position = { world.x, world.y, world.z };
            m_CPUHead->Color    = color;
            m_CPUHead->TexCoord = uvs[i];
            m_CPUHead->TexIndex = texIndex;
            m_CPUHead->Tiling   = tiling;
            m_CPUHead++;
        }

        m_CPUIndexCount += 6;
    }

    void BatchRenderer2D::EmitQuadInstanced(const glm::mat4& transform, GLuint texID,
                                           const glm::vec2& uvMin, const glm::vec2& uvMax,
                                           const glm::vec4& color, float tiling)
    {
        if (m_InstanceCount + 1 > MaxInstances)
            NextBatch();

        const float texIndex = (float)AcquireTextureSlot(texID);

        QuadInstance inst{};
        inst.Transform = transform;
        inst.Color = color;
        inst.UVMinMax = { uvMin.x, uvMin.y, uvMax.x, uvMax.y };
        inst.Tex = { texIndex, tiling };

        *m_InstHead = inst;
        m_InstHead++;
        m_InstanceCount++;
    }

    void BatchRenderer2D::Flush()
    {
        m_LastBoundVAO = 0;
        m_LastBoundProgram = 0;

        if (m_Mode == Mode::CPUExpanded)
        {
            if (m_CPUIndexCount == 0) return;
            UploadAndDrawCPU();
        }
        else
        {
            if (m_InstanceCount == 0) return;
            UploadAndDrawInstanced();
        }

        m_Stats.Flushes++;
    }

    void BatchRenderer2D::UploadAndDrawCPU()
    {
        const uint32_t bytes = (uint32_t)((uint8_t*)m_CPUHead - (uint8_t*)m_CPUBase);
        m_VB_CPU.SetData(m_CPUBase, bytes, 0);

        // Bind material + UBO + textures
        GLuint prog = m_Material->GetProgram();
        UseProgram(m_LastBoundProgram, prog);

        if (m_FrameUBO.GetRendererID() != 0)
            m_FrameUBO.BindBase();

        // bind textures to units
        for (uint32_t i = 0; i < (uint32_t)m_TextureSlots.size(); i++)
            glBindTextureUnit(i, m_TextureSlots[i]);

        // draw
        BindVAO(m_LastBoundVAO, m_VAO_CPU);
        glDrawElements(GL_TRIANGLES, (GLsizei)m_CPUIndexCount, GL_UNSIGNED_INT, nullptr);

        m_Stats.DrawCalls++;
    }

    void BatchRenderer2D::UploadAndDrawInstanced()
    {
        const uint32_t bytes = (uint32_t)((uint8_t*)m_InstHead - (uint8_t*)m_InstBase);
        m_VB_Instance.SetData(m_InstBase, bytes, 0);

        GLuint prog = m_Material->GetProgram();
        UseProgram(m_LastBoundProgram, prog);

        if (m_FrameUBO.GetRendererID() != 0)
            m_FrameUBO.BindBase();

        for (uint32_t i = 0; i < (uint32_t)m_TextureSlots.size(); i++)
            glBindTextureUnit(i, m_TextureSlots[i]);

        BindVAO(m_LastBoundVAO, m_VAO_Inst);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (GLsizei)m_InstanceCount);

        m_Stats.DrawCalls++;
    }

    void BatchRenderer2D::FlushSorted()
    {
        if (m_SortedCmds.empty())
            return;

        // Sort by texture (simple + effective)
        std::sort(m_SortedCmds.begin(), m_SortedCmds.end(),
            [](const SpriteCmd& a, const SpriteCmd& b) { return a.TextureID < b.TextureID; });

        StartBatch();

        for (const auto& cmd : m_SortedCmds)
        {
            if (m_Mode == Mode::CPUExpanded)
                EmitQuadCPU(cmd.Transform, cmd.TextureID, cmd.UVMin, cmd.UVMax, cmd.Color, cmd.Tiling);
            else
                EmitQuadInstanced(cmd.Transform, cmd.TextureID, cmd.UVMin, cmd.UVMax, cmd.Color, cmd.Tiling);

            m_Stats.QuadCount++;
        }

        m_SortedCmds.clear();
        Flush();
    }
}
