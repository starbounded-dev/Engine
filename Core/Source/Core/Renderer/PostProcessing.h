#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Core::Renderer
{
    class RenderTarget;
    class Material;

    // Base post-processing effect
    class PostProcessEffect
    {
    public:
        PostProcessEffect() = default;
        virtual ~PostProcessEffect() = default;

        virtual void Apply(RenderTarget* source, RenderTarget* destination) = 0;

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        void SetName(const std::string& name) { m_Name = name; }
        const std::string& GetName() const { return m_Name; }

    protected:
        bool m_Enabled = true;
        std::string m_Name;
    };

    // Fullscreen quad for post-processing
    class FullscreenQuad
    {
    public:
        FullscreenQuad();
        ~FullscreenQuad();

        FullscreenQuad(const FullscreenQuad&) = delete;
        FullscreenQuad& operator=(const FullscreenQuad&) = delete;

        void Draw() const;

    private:
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
    };

    // Bloom effect
    class BloomEffect : public PostProcessEffect
    {
    public:
        BloomEffect();
        ~BloomEffect();

        void Apply(RenderTarget* source, RenderTarget* destination) override;

        void SetThreshold(float threshold) { m_Threshold = threshold; }
        float GetThreshold() const { return m_Threshold; }

        void SetIntensity(float intensity) { m_Intensity = intensity; }
        float GetIntensity() const { return m_Intensity; }

        void SetBlurPasses(int passes) { m_BlurPasses = passes; }
        int GetBlurPasses() const { return m_BlurPasses; }

    private:
        void InitializeResources();

    private:
        float m_Threshold = 1.0f;
        float m_Intensity = 0.5f;
        int m_BlurPasses = 5;

        GLuint m_BrightFilterShader = 0;
        GLuint m_BlurShader = 0;
        GLuint m_CompositeShader = 0;

        std::unique_ptr<RenderTarget> m_BrightTarget;
        std::unique_ptr<RenderTarget> m_BlurTarget1;
        std::unique_ptr<RenderTarget> m_BlurTarget2;

        std::unique_ptr<FullscreenQuad> m_Quad;
    };

    // Tone mapping effect
    class ToneMappingEffect : public PostProcessEffect
    {
    public:
        enum class Mode
        {
            None = 0,
            Reinhard,
            ReinhardLuminance,
            ACES,
            Uncharted2
        };

        ToneMappingEffect();
        ~ToneMappingEffect();

        void Apply(RenderTarget* source, RenderTarget* destination) override;

        void SetMode(Mode mode) { m_Mode = mode; }
        Mode GetMode() const { return m_Mode; }

        void SetExposure(float exposure) { m_Exposure = exposure; }
        float GetExposure() const { return m_Exposure; }

        void SetGamma(float gamma) { m_Gamma = gamma; }
        float GetGamma() const { return m_Gamma; }

    private:
        void InitializeResources();

    private:
        Mode m_Mode = Mode::ACES;
        float m_Exposure = 1.0f;
        float m_Gamma = 2.2f;

        GLuint m_ToneMappingShader = 0;
        std::unique_ptr<FullscreenQuad> m_Quad;
    };

    // FXAA (Fast Approximate Anti-Aliasing) effect
    class FXAAEffect : public PostProcessEffect
    {
    public:
        FXAAEffect();
        ~FXAAEffect();

        void Apply(RenderTarget* source, RenderTarget* destination) override;

        void SetQualitySubpix(float value) { m_QualitySubpix = value; }
        float GetQualitySubpix() const { return m_QualitySubpix; }

        void SetQualityEdgeThreshold(float value) { m_QualityEdgeThreshold = value; }
        float GetQualityEdgeThreshold() const { return m_QualityEdgeThreshold; }

    private:
        void InitializeResources();

    private:
        float m_QualitySubpix = 0.75f;
        float m_QualityEdgeThreshold = 0.125f;

        GLuint m_FXAAShader = 0;
        std::unique_ptr<FullscreenQuad> m_Quad;
    };

    // Post-processing stack
    class PostProcessingStack
    {
    public:
        PostProcessingStack() = default;

        void AddEffect(std::shared_ptr<PostProcessEffect> effect);
        void RemoveEffect(const std::string& name);
        void ClearEffects();

        std::shared_ptr<PostProcessEffect> GetEffect(const std::string& name) const;

        // Apply all enabled effects
        void Apply(RenderTarget* source, RenderTarget* destination);

        size_t GetEffectCount() const { return m_Effects.size(); }

    private:
        std::vector<std::shared_ptr<PostProcessEffect>> m_Effects;
    };
}
