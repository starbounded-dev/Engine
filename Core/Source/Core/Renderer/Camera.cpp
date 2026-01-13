#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Core::Renderer
{
    static float DegToRad(float deg) { return deg * glm::pi<float>() / 180.0f; }
    static float RadToDeg(float rad) { return rad * 180.0f / glm::pi<float>(); }

    // ---------------- Camera ----------------

    Camera::Camera()
    {
        RecalcProjection();
        RecalcView();
    }

    void Camera::SetPerspective(float fovDegrees, float aspect, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_PerspectiveFOVDegrees = fovDegrees;
        m_Aspect = aspect;
        m_Near = nearClip;
        m_Far = farClip;
        m_ProjDirty = true;
    }

    void Camera::SetOrthographic(float size, float aspect, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoSize = std::max(0.0001f, size);
        m_Aspect = aspect;
        m_Near = nearClip;
        m_Far = farClip;
        m_OrthoUsingExplicitBounds = false;
        m_ProjDirty = true;
    }

    void Camera::SetOrthographicBounds(float left, float right, float bottom, float top, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoLeft = left; m_OrthoRight = right; m_OrthoBottom = bottom; m_OrthoTop = top;
        m_Near = nearClip;
        m_Far = farClip;
        m_OrthoUsingExplicitBounds = true;
        m_ProjDirty = true;
    }

    void Camera::SetViewportSize(float width, float height)
    {
        if (height <= 0.0f) height = 1.0f;
        m_Aspect = width / height;
        m_ProjDirty = true;
    }

    const glm::mat4& Camera::GetProjectionMatrix() const
    {
        if (m_ProjDirty)
            RecalcProjection();
        return m_Projection;
    }

    const glm::mat4& Camera::GetViewMatrix() const
    {
        if (m_ViewDirty)
            RecalcView();
        return m_View;
    }

    glm::mat4 Camera::GetViewProjectionMatrix() const
    {
        return GetProjectionMatrix() * GetViewMatrix();
    }

    void Camera::SetPosition(const glm::vec3& pos)
    {
        m_Position = pos;
        m_ViewDirty = true;
    }

    void Camera::SetRotationRadians(const glm::vec3& eulerRadians)
    {
        m_RotationRadians = eulerRadians;
        m_UseLookAt = false;
        m_ViewDirty = true;
    }

    void Camera::SetRotationDegrees(const glm::vec3& eulerDegrees)
    {
        SetRotationRadians(glm::vec3(DegToRad(eulerDegrees.x), DegToRad(eulerDegrees.y), DegToRad(eulerDegrees.z)));
    }

    void Camera::LookAt(const glm::vec3& target, const glm::vec3& up)
    {
        m_UseLookAt = true;
        m_LookAtTarget = target;
        m_LookAtUp = up;
        m_ViewDirty = true;
    }

    void Camera::ClearLookAt()
    {
        m_UseLookAt = false;
        m_ViewDirty = true;
    }

    glm::vec3 Camera::GetForward() const
    {
        // From Euler: pitch (x), yaw (y), roll (z)
        const glm::quat q = glm::quat(m_RotationRadians);
        return glm::normalize(q * glm::vec3(0, 0, -1));
    }

    glm::vec3 Camera::GetRight() const
    {
        const glm::quat q = glm::quat(m_RotationRadians);
        return glm::normalize(q * glm::vec3(1, 0, 0));
    }

    glm::vec3 Camera::GetUp() const
    {
        const glm::quat q = glm::quat(m_RotationRadians);
        return glm::normalize(q * glm::vec3(0, 1, 0));
    }

    void Camera::RecalcProjection() const
    {
        if (m_ProjectionType == ProjectionType::Perspective)
        {
            m_Projection = glm::perspective(glm::radians(m_PerspectiveFOVDegrees), m_Aspect, m_Near, m_Far);
        }
        else
        {
            if (m_OrthoUsingExplicitBounds)
            {
                m_Projection = glm::ortho(m_OrthoLeft, m_OrthoRight, m_OrthoBottom, m_OrthoTop, m_Near, m_Far);
            }
            else
            {
                // size is vertical half-size
                const float halfH = m_OrthoSize;
                const float halfW = m_OrthoSize * m_Aspect;
                m_Projection = glm::ortho(-halfW, halfW, -halfH, halfH, m_Near, m_Far);
            }
        }

        m_ProjDirty = false;
    }

    void Camera::RecalcView() const
    {
        if (m_UseLookAt)
        {
            m_View = glm::lookAt(m_Position, m_LookAtTarget, m_LookAtUp);
        }
        else
        {
            // View = inverse( T * R )
            const glm::quat q = glm::quat(m_RotationRadians);
            const glm::mat4 rot = glm::toMat4(q);
            const glm::mat4 tr = glm::translate(glm::mat4(1.0f), m_Position);
            m_View = glm::inverse(tr * rot);
        }

        m_ViewDirty = false;
    }

    // ---------------- FPSCameraController ----------------

    FPSCameraController::FPSCameraController(Camera* camera)
        : m_Camera(camera)
    {
        // Initialize camera rotation from yaw/pitch
        SetYawPitchDegrees(m_YawDeg, m_PitchDeg);
    }

    void FPSCameraController::SetYawPitchDegrees(float yawDeg, float pitchDeg)
    {
        m_YawDeg = yawDeg;
        m_PitchDeg = std::clamp(pitchDeg, -89.9f, 89.9f);

        // pitch=x, yaw=y, roll=z
        m_Camera->SetRotationDegrees({ m_PitchDeg, m_YawDeg, 0.0f });
    }

    void FPSCameraController::OnUpdate(float ts, const CameraInputState& input)
    {
        if (!m_Camera) return;

        // Look (mouse)
        if (input.Look)
        {
            m_YawDeg   += input.MouseDeltaX * m_MouseSensitivityDegPerPixel;
            m_PitchDeg -= input.MouseDeltaY * m_MouseSensitivityDegPerPixel; // invert Y for typical FPS
            m_PitchDeg = std::clamp(m_PitchDeg, -89.9f, 89.9f);
            m_Camera->SetRotationDegrees({ m_PitchDeg, m_YawDeg, 0.0f });
        }

        // Move
        float speed = m_MoveSpeed * (input.Fast ? m_FastMultiplier : 1.0f);

        glm::vec3 pos = m_Camera->GetPosition();
        const glm::vec3 fwd = m_Camera->GetForward();
        const glm::vec3 right = m_Camera->GetRight();
        const glm::vec3 up = glm::vec3(0, 1, 0);

        if (input.Forward)  pos += fwd * speed * ts;
        if (input.Backward) pos -= fwd * speed * ts;
        if (input.Right)    pos += right * speed * ts;
        if (input.Left)     pos -= right * speed * ts;
        if (input.Up)       pos += up * speed * ts;
        if (input.Down)     pos -= up * speed * ts;

        m_Camera->SetPosition(pos);
        m_Camera->ClearLookAt(); // FPS is free-look
    }

    // ---------------- OrbitCameraController ----------------

    OrbitCameraController::OrbitCameraController(Camera* camera)
        : m_Camera(camera)
    {
        RebuildCameraTransform();
    }

    void OrbitCameraController::OnUpdate(float /*ts*/, const CameraInputState& input)
    {
        if (!m_Camera) return;

        // Zoom
        if (input.ScrollDelta != 0.0f)
        {
            m_Distance -= input.ScrollDelta * m_ZoomSpeed;
            m_Distance = std::max(0.05f, m_Distance);
        }

        // Rotate (RMB)
        if (input.Look)
        {
            m_YawDeg   += input.MouseDeltaX * m_RotateSensitivityDegPerPixel;
            m_PitchDeg -= input.MouseDeltaY * m_RotateSensitivityDegPerPixel;
            m_PitchDeg = std::clamp(m_PitchDeg, -89.0f, 89.0f);
        }

        // Pan (MMB)
        if (input.Pan)
        {
            // Pan in camera plane (scaled by distance so it feels consistent)
            const float panScale = m_Distance * m_PanSensitivityUnitsPerPixel;

            // build a temporary orientation from yaw/pitch
            glm::quat q = glm::quat(glm::vec3(DegToRad(m_PitchDeg), DegToRad(m_YawDeg), 0.0f));
            glm::vec3 right = glm::normalize(q * glm::vec3(1, 0, 0));
            glm::vec3 up    = glm::normalize(q * glm::vec3(0, 1, 0));

            m_Target -= right * (input.MouseDeltaX * panScale);
            m_Target += up    * (input.MouseDeltaY * panScale);
        }

        RebuildCameraTransform();
    }

    void OrbitCameraController::RebuildCameraTransform()
    {
        glm::quat q = glm::quat(glm::vec3(DegToRad(m_PitchDeg), DegToRad(m_YawDeg), 0.0f));
        glm::vec3 forward = glm::normalize(q * glm::vec3(0, 0, -1));

        // Camera sits back along forward axis
        glm::vec3 pos = m_Target - forward * m_Distance;

        m_Camera->SetPosition(pos);
        m_Camera->LookAt(m_Target);
    }
}
