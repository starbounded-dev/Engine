#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace Core::Renderer
{
    enum class ProjectionType : uint8_t
    {
        Perspective = 0,
        Orthographic
    };

    // Camera "component" (position + rotation + projection)
    class Camera
    {
    public:
        Camera();

        // ---- Projection ----
        void SetPerspective(float fovDegrees, float aspect, float nearClip, float farClip);
        void SetOrthographic(float size, float aspect, float nearClip, float farClip); // size = vertical half-size (like ortho zoom)
        void SetOrthographicBounds(float left, float right, float bottom, float top, float nearClip, float farClip);

        void SetViewportSize(float width, float height);

        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; m_ProjDirty = true; }
        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        float GetFOVDegrees() const { return m_PerspectiveFOVDegrees; }
        float GetAspect() const { return m_Aspect; }
        float GetNearClip() const { return m_Near; }
        float GetFarClip() const { return m_Far; }

        float GetOrthoSize() const { return m_OrthoSize; }

        const glm::mat4& GetProjectionMatrix() const;
        const glm::mat4& GetViewMatrix() const;
        glm::mat4 GetViewProjectionMatrix() const;

        // ---- Transform ----
        void SetPosition(const glm::vec3& pos);
        const glm::vec3& GetPosition() const { return m_Position; }

        // Rotation stored as Euler radians: (pitch, yaw, roll)
        void SetRotationRadians(const glm::vec3& eulerRadians);
        void SetRotationDegrees(const glm::vec3& eulerDegrees);
        const glm::vec3& GetRotationRadians() const { return m_RotationRadians; }

        // Convenience for "look at"
        void LookAt(const glm::vec3& target, const glm::vec3& up = {0, 1, 0});
        void ClearLookAt();

        // Directions (from current rotation)
        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

    private:
        void RecalcProjection() const;
        void RecalcView() const;

    private:
        // projection
        ProjectionType m_ProjectionType = ProjectionType::Perspective;
        float m_Aspect = 16.0f / 9.0f;
        float m_Near = 0.1f;
        float m_Far = 1000.0f;

        // perspective
        float m_PerspectiveFOVDegrees = 45.0f;

        // orthographic
        float m_OrthoSize = 5.0f; // vertical half-size
        bool  m_OrthoUsingExplicitBounds = false;
        float m_OrthoLeft = -1.0f, m_OrthoRight = 1.0f, m_OrthoBottom = -1.0f, m_OrthoTop = 1.0f;

        // transform
        glm::vec3 m_Position{0.0f, 0.0f, 5.0f};
        glm::vec3 m_RotationRadians{0.0f}; // pitch, yaw, roll

        bool m_UseLookAt = false;
        glm::vec3 m_LookAtTarget{0.0f};
        glm::vec3 m_LookAtUp{0.0f, 1.0f, 0.0f};

        // cached matrices
        mutable bool m_ProjDirty = true;
        mutable bool m_ViewDirty = true;
        mutable glm::mat4 m_Projection{1.0f};
        mutable glm::mat4 m_View{1.0f};
    };

    // Input bundle you fill each frame (map it from Core::Input however you want)
    struct CameraInputState
    {
        // keyboard
        bool Forward = false;   // W
        bool Backward = false;  // S
        bool Left = false;      // A
        bool Right = false;     // D
        bool Up = false;        // E / Space
        bool Down = false;      // Q / Ctrl
        bool Fast = false;      // Shift

        // mouse buttons
        bool Look = false;      // RMB (FPS look / Orbit rotate)
        bool Pan = false;       // MMB (Orbit pan)

        // mouse deltas (pixels) and wheel
        float MouseDeltaX = 0.0f;
        float MouseDeltaY = 0.0f;
        float ScrollDelta = 0.0f;

        void ResetDeltas()
        {
            MouseDeltaX = MouseDeltaY = 0.0f;
            ScrollDelta = 0.0f;
        }
    };

    class FPSCameraController
    {
    public:
        explicit FPSCameraController(Camera* camera);

        void SetMoveSpeed(float unitsPerSecond) { m_MoveSpeed = unitsPerSecond; }
        void SetFastMultiplier(float mult) { m_FastMultiplier = mult; }
        void SetMouseSensitivity(float degPerPixel) { m_MouseSensitivityDegPerPixel = degPerPixel; }

        void OnUpdate(float ts, const CameraInputState& input);

        float GetYawDegrees() const { return m_YawDeg; }
        float GetPitchDegrees() const { return m_PitchDeg; }

        void SetYawPitchDegrees(float yawDeg, float pitchDeg);

    private:
        Camera* m_Camera = nullptr;

        float m_MoveSpeed = 6.0f;
        float m_FastMultiplier = 2.5f;

        float m_MouseSensitivityDegPerPixel = 0.08f;

        float m_YawDeg = -90.0f;   // look forward on -Z by default
        float m_PitchDeg = 0.0f;
    };

    class OrbitCameraController
    {
    public:
        explicit OrbitCameraController(Camera* camera);

        void SetTarget(const glm::vec3& target) { m_Target = target; }
        const glm::vec3& GetTarget() const { return m_Target; }

        void SetDistance(float d) { m_Distance = (d < 0.05f) ? 0.05f : d; }
        float GetDistance() const { return m_Distance; }

        void SetRotateSensitivity(float degPerPixel) { m_RotateSensitivityDegPerPixel = degPerPixel; }
        void SetPanSensitivity(float unitsPerPixel) { m_PanSensitivityUnitsPerPixel = unitsPerPixel; }
        void SetZoomSpeed(float unitsPerScroll) { m_ZoomSpeed = unitsPerScroll; }

        void OnUpdate(float ts, const CameraInputState& input);

    private:
        void RebuildCameraTransform();

    private:
        Camera* m_Camera = nullptr;

        glm::vec3 m_Target{0.0f, 0.0f, 0.0f};
        float m_Distance = 5.0f;

        float m_YawDeg = 0.0f;
        float m_PitchDeg = 20.0f;

        float m_RotateSensitivityDegPerPixel = 0.12f;
        float m_PanSensitivityUnitsPerPixel = 0.005f;
        float m_ZoomSpeed = 0.6f;
    };
}
