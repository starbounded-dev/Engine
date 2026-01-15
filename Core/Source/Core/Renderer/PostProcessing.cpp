#include "PostProcessing.h"
#include "RenderPass.h"
#include "Shader.h"
#include <algorithm>

namespace Core::Renderer
{
    // FullscreenQuad implementation
    FullscreenQuad::FullscreenQuad()
    {
        // Fullscreen quad vertices (NDC space)
        float vertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // TexCoord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    FullscreenQuad::~FullscreenQuad()
    {
        if (m_VAO)
            glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO)
            glDeleteBuffers(1, &m_VBO);
    }

    void FullscreenQuad::Draw() const
    {
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // BloomEffect implementation
    BloomEffect::BloomEffect()
    {
        SetName("Bloom");
        InitializeResources();
    }

    BloomEffect::~BloomEffect()
    {
        if (m_BrightFilterShader != 0)
            glDeleteProgram(m_BrightFilterShader);
        if (m_BlurShader != 0)
            glDeleteProgram(m_BlurShader);
        if (m_CompositeShader != 0)
            glDeleteProgram(m_CompositeShader);
    }

    void BloomEffect::InitializeResources()
    {
        m_Quad = std::make_unique<FullscreenQuad>();
        
        // Note: In a real implementation, you would load actual shader files here
        // For now, these are placeholders that would need proper shader source
        // m_BrightFilterShader = CreateGraphicsShader(...);
        // m_BlurShader = CreateGraphicsShader(...);
        // m_CompositeShader = CreateGraphicsShader(...);
    }

    void BloomEffect::Apply(RenderTarget* source, RenderTarget* destination)
    {
        if (!m_Enabled || !source)
            return;

        // Placeholder implementation
        // 1. Extract bright pixels (threshold)
        // 2. Blur the bright texture multiple times
        // 3. Composite with original image
        
        // For now, just copy source to destination
        // A full implementation would do the bloom passes
    }

    // ToneMappingEffect implementation
    ToneMappingEffect::ToneMappingEffect()
    {
        SetName("ToneMapping");
        InitializeResources();
    }

    ToneMappingEffect::~ToneMappingEffect()
    {
        if (m_ToneMappingShader != 0)
            glDeleteProgram(m_ToneMappingShader);
    }

    void ToneMappingEffect::InitializeResources()
    {
        m_Quad = std::make_unique<FullscreenQuad>();
        
        // Note: In a real implementation, load the tone mapping shader
        // m_ToneMappingShader = CreateGraphicsShader(...);
    }

    void ToneMappingEffect::Apply(RenderTarget* source, RenderTarget* destination)
    {
        if (!m_Enabled || !source || !m_ToneMappingShader)
            return;

        // Bind destination
        if (destination)
            destination->Bind();
        else
            RenderTarget::Unbind();

        // Use tone mapping shader
        glUseProgram(m_ToneMappingShader);
        
        // Set uniforms
        GLint exposureLoc = glGetUniformLocation(m_ToneMappingShader, "u_Exposure");
        GLint gammaLoc = glGetUniformLocation(m_ToneMappingShader, "u_Gamma");
        GLint modeLoc = glGetUniformLocation(m_ToneMappingShader, "u_Mode");
        
        if (exposureLoc != -1)
            glUniform1f(exposureLoc, m_Exposure);
        if (gammaLoc != -1)
            glUniform1f(gammaLoc, m_Gamma);
        if (modeLoc != -1)
            glUniform1i(modeLoc, static_cast<int>(m_Mode));

        // Bind source texture
        source->BindColorAttachment(0, 0);

        // Draw fullscreen quad
        m_Quad->Draw();
    }

    // FXAAEffect implementation
    FXAAEffect::FXAAEffect()
    {
        SetName("FXAA");
        InitializeResources();
    }

    FXAAEffect::~FXAAEffect()
    {
        if (m_FXAAShader != 0)
            glDeleteProgram(m_FXAAShader);
    }

    void FXAAEffect::InitializeResources()
    {
        m_Quad = std::make_unique<FullscreenQuad>();
        
        // Note: Load FXAA shader in real implementation
        // m_FXAAShader = CreateGraphicsShader(...);
    }

    void FXAAEffect::Apply(RenderTarget* source, RenderTarget* destination)
    {
        if (!m_Enabled || !source || !m_FXAAShader)
            return;

        // Bind destination
        if (destination)
            destination->Bind();
        else
            RenderTarget::Unbind();

        // Use FXAA shader
        glUseProgram(m_FXAAShader);
        
        // Set uniforms
        GLint subpixLoc = glGetUniformLocation(m_FXAAShader, "u_QualitySubpix");
        GLint edgeThresholdLoc = glGetUniformLocation(m_FXAAShader, "u_QualityEdgeThreshold");
        GLint resolutionLoc = glGetUniformLocation(m_FXAAShader, "u_Resolution");
        
        if (subpixLoc != -1)
            glUniform1f(subpixLoc, m_QualitySubpix);
        if (edgeThresholdLoc != -1)
            glUniform1f(edgeThresholdLoc, m_QualityEdgeThreshold);
        if (resolutionLoc != -1)
            glUniform2f(resolutionLoc, static_cast<float>(source->GetWidth()), 
                       static_cast<float>(source->GetHeight()));

        // Bind source texture
        source->BindColorAttachment(0, 0);

        // Draw fullscreen quad
        m_Quad->Draw();
    }

    // PostProcessingStack implementation
    void PostProcessingStack::AddEffect(std::shared_ptr<PostProcessEffect> effect)
    {
        if (effect)
        {
            m_Effects.push_back(effect);
        }
    }

    void PostProcessingStack::RemoveEffect(const std::string& name)
    {
        auto it = std::find_if(m_Effects.begin(), m_Effects.end(),
            [&name](const std::shared_ptr<PostProcessEffect>& effect) {
                return effect->GetName() == name;
            });

        if (it != m_Effects.end())
        {
            m_Effects.erase(it);
        }
    }

    void PostProcessingStack::ClearEffects()
    {
        m_Effects.clear();
    }

    std::shared_ptr<PostProcessEffect> PostProcessingStack::GetEffect(const std::string& name) const
    {
        auto it = std::find_if(m_Effects.begin(), m_Effects.end(),
            [&name](const std::shared_ptr<PostProcessEffect>& effect) {
                return effect->GetName() == name;
            });

        if (it != m_Effects.end())
        {
            return *it;
        }
        return nullptr;
    }

    void PostProcessingStack::Apply(RenderTarget* source, RenderTarget* destination)
    {
        if (m_Effects.empty() || !source)
            return;

        // For now, apply each effect sequentially
        // In a more advanced implementation, you might ping-pong between intermediate buffers
        RenderTarget* currentSource = source;
        
        for (size_t i = 0; i < m_Effects.size(); ++i)
        {
            auto& effect = m_Effects[i];
            if (effect && effect->IsEnabled())
            {
                // Last effect writes to destination
                RenderTarget* currentDest = (i == m_Effects.size() - 1) ? destination : nullptr;
                effect->Apply(currentSource, currentDest);
            }
        }
    }
}
