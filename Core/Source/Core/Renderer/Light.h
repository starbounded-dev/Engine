#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

namespace Core::Renderer
{
    enum class LightType : uint8_t
    {
        Directional = 0,
        Point,
        Spot
    };

    // Base light data structure for UBO/SSBO
    struct LightData
    {
        glm::vec4 PositionAndType; // xyz = position, w = type (0=directional, 1=point, 2=spot)
        glm::vec4 DirectionAndRange; // xyz = direction, w = range
        glm::vec4 ColorAndIntensity; // rgb = color, a = intensity
        glm::vec4 SpotAngles; // x = inner cone angle (cos), y = outer cone angle (cos), zw = padding
        glm::vec4 Attenuation; // x = constant, y = linear, z = quadratic, w = padding
        glm::mat4 ViewProjection; // For shadow mapping
        glm::ivec4 Flags; // x = castsShadows, y = enabled, zw = padding
    };

    class Light
    {
    public:
        Light() = default;
        Light(LightType type);

        void SetType(LightType type) { m_Type = type; }
        LightType GetType() const { return m_Type; }

        void SetPosition(const glm::vec3& position) { m_Position = position; }
        const glm::vec3& GetPosition() const { return m_Position; }

        void SetDirection(const glm::vec3& direction) { m_Direction = glm::normalize(direction); }
        const glm::vec3& GetDirection() const { return m_Direction; }

        void SetColor(const glm::vec3& color) { m_Color = color; }
        const glm::vec3& GetColor() const { return m_Color; }

        void SetIntensity(float intensity) { m_Intensity = intensity; }
        float GetIntensity() const { return m_Intensity; }

        void SetRange(float range) { m_Range = range; }
        float GetRange() const { return m_Range; }

        // Spot light specific
        void SetSpotAngles(float innerDegrees, float outerDegrees);
        float GetSpotInnerAngle() const { return m_SpotInnerAngleCos; }
        float GetSpotOuterAngle() const { return m_SpotOuterAngleCos; }

        // Attenuation (for point and spot lights)
        void SetAttenuation(float constant, float linear, float quadratic);
        void GetAttenuation(float& constant, float& linear, float& quadratic) const;

        // Shadow mapping
        void SetCastsShadows(bool casts) { m_CastsShadows = casts; }
        bool GetCastsShadows() const { return m_CastsShadows; }

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        // Calculate view-projection matrix for shadow mapping
        glm::mat4 CalculateShadowViewProjection(float nearPlane = 0.1f, float farPlane = 100.0f) const;

        // Convert to GPU-friendly data structure
        LightData ToLightData() const;

    private:
        LightType m_Type = LightType::Directional;
        glm::vec3 m_Position{0.0f, 5.0f, 0.0f};
        glm::vec3 m_Direction{0.0f, -1.0f, 0.0f};
        glm::vec3 m_Color{1.0f, 1.0f, 1.0f};
        float m_Intensity = 1.0f;
        float m_Range = 10.0f;

        // Spot light parameters (stored as cosine for shader efficiency)
        float m_SpotInnerAngleCos = 0.9f; // ~25 degrees
        float m_SpotOuterAngleCos = 0.82f; // ~35 degrees

        // Attenuation parameters
        float m_AttenuationConstant = 1.0f;
        float m_AttenuationLinear = 0.09f;
        float m_AttenuationQuadratic = 0.032f;

        bool m_CastsShadows = false;
        bool m_Enabled = true;
    };

    // Light manager for scene lighting
    class LightManager
    {
    public:
        LightManager() = default;

        void Clear();
        
        uint32_t AddLight(const Light& light);
        void RemoveLight(uint32_t index);
        void UpdateLight(uint32_t index, const Light& light);

        Light* GetLight(uint32_t index);
        const Light* GetLight(uint32_t index) const;

        size_t GetLightCount() const { return m_Lights.size(); }
        const std::vector<Light>& GetLights() const { return m_Lights; }

        // Get all lights as GPU data
        std::vector<LightData> GetLightDataArray() const;

        // Ambient light for the scene
        void SetAmbientLight(const glm::vec3& ambient) { m_AmbientLight = ambient; }
        const glm::vec3& GetAmbientLight() const { return m_AmbientLight; }

    private:
        std::vector<Light> m_Lights;
        glm::vec3 m_AmbientLight{0.1f, 0.1f, 0.1f};
    };
}
