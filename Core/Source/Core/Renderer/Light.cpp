#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Core::Renderer
{
    Light::Light(LightType type)
        : m_Type(type)
    {
        if (type == LightType::Directional)
        {
            m_Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        }
    }

    void Light::SetSpotAngles(float innerDegrees, float outerDegrees)
    {
        m_SpotInnerAngleCos = std::cos(glm::radians(innerDegrees));
        m_SpotOuterAngleCos = std::cos(glm::radians(outerDegrees));
    }

    void Light::SetAttenuation(float constant, float linear, float quadratic)
    {
        m_AttenuationConstant = constant;
        m_AttenuationLinear = linear;
        m_AttenuationQuadratic = quadratic;
    }

    void Light::GetAttenuation(float& constant, float& linear, float& quadratic) const
    {
        constant = m_AttenuationConstant;
        linear = m_AttenuationLinear;
        quadratic = m_AttenuationQuadratic;
    }

    glm::mat4 Light::CalculateShadowViewProjection(float nearPlane, float farPlane) const
    {
        glm::mat4 view(1.0f);
        glm::mat4 projection(1.0f);

        switch (m_Type)
        {
        case LightType::Directional:
        {
            // For directional light, use orthographic projection
            glm::vec3 target = m_Position + m_Direction;
            view = glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));
            
            // Use a large orthographic box for directional shadows
            float orthoSize = 20.0f;
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
            break;
        }
        case LightType::Point:
        {
            // Point lights need cubemap shadows (6 faces)
            // For now, just use one face as perspective
            glm::vec3 target = m_Position + glm::vec3(0.0f, 0.0f, -1.0f);
            view = glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));
            projection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, m_Range);
            break;
        }
        case LightType::Spot:
        {
            glm::vec3 target = m_Position + m_Direction;
            view = glm::lookAt(m_Position, target, glm::vec3(0.0f, 1.0f, 0.0f));
            
            // Use outer angle for projection FOV
            float fov = std::acos(m_SpotOuterAngleCos) * 2.0f;
            projection = glm::perspective(fov, 1.0f, nearPlane, m_Range);
            break;
        }
        }

        return projection * view;
    }

    LightData Light::ToLightData() const
    {
        LightData data{};
        
        data.PositionAndType = glm::vec4(m_Position, static_cast<float>(m_Type));
        data.DirectionAndRange = glm::vec4(m_Direction, m_Range);
        data.ColorAndIntensity = glm::vec4(m_Color, m_Intensity);
        data.SpotAngles = glm::vec4(m_SpotInnerAngleCos, m_SpotOuterAngleCos, 0.0f, 0.0f);
        data.Attenuation = glm::vec4(m_AttenuationConstant, m_AttenuationLinear, m_AttenuationQuadratic, 0.0f);
        data.ViewProjection = CalculateShadowViewProjection();
        data.Flags = glm::ivec4(m_CastsShadows ? 1 : 0, m_Enabled ? 1 : 0, 0, 0);

        return data;
    }

    // LightManager implementation
    void LightManager::Clear()
    {
        m_Lights.clear();
    }

    uint32_t LightManager::AddLight(const Light& light)
    {
        m_Lights.push_back(light);
        return static_cast<uint32_t>(m_Lights.size() - 1);
    }

    void LightManager::RemoveLight(uint32_t index)
    {
        if (index < m_Lights.size())
        {
            m_Lights.erase(m_Lights.begin() + index);
        }
    }

    void LightManager::UpdateLight(uint32_t index, const Light& light)
    {
        if (index < m_Lights.size())
        {
            m_Lights[index] = light;
        }
    }

    Light* LightManager::GetLight(uint32_t index)
    {
        if (index < m_Lights.size())
        {
            return &m_Lights[index];
        }
        return nullptr;
    }

    const Light* LightManager::GetLight(uint32_t index) const
    {
        if (index < m_Lights.size())
        {
            return &m_Lights[index];
        }
        return nullptr;
    }

    std::vector<LightData> LightManager::GetLightDataArray() const
    {
        std::vector<LightData> lightData;
        lightData.reserve(m_Lights.size());

        for (const auto& light : m_Lights)
        {
            if (light.IsEnabled())
            {
                lightData.push_back(light.ToLightData());
            }
        }

        return lightData;
    }
}
