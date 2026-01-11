#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core::Renderer {

	enum class CameraProjectionType
	{
		Perspective,
		Orthographic
	};

	class Camera
	{
	public:
		Camera();
		~Camera() = default;

		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;

		// Perspective camera
		Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

		// Orthographic camera
		Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane);

		// View matrix
		void SetPosition(const glm::vec3& position);
		void SetRotation(const glm::vec3& rotation); // Euler angles (pitch, yaw, roll)
		void SetTarget(const glm::vec3& target);
		void SetUp(const glm::vec3& up);

		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetTarget() const { return m_Target; }
		inline const glm::vec3& GetUp() const { return m_Up; }

		// Projection matrix
		void SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
		void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
		void SetProjectionType(CameraProjectionType type);

		// Get matrices
		const glm::mat4& GetViewMatrix() const;
		const glm::mat4& GetProjectionMatrix() const;
		glm::mat4 GetViewProjectionMatrix() const;

		// Recalculate matrices (call when properties change)
		void RecalculateViewMatrix() const;
		void RecalculateProjectionMatrix() const;

		// Perspective properties
		inline float GetFOV() const { return m_FOV; }
		inline float GetAspectRatio() const { return m_AspectRatio; }
		inline float GetNearPlane() const { return m_NearPlane; }
		inline float GetFarPlane() const { return m_FarPlane; }

		// Orthographic properties
		inline float GetLeft() const { return m_Left; }
		inline float GetRight() const { return m_Right; }
		inline float GetBottom() const { return m_Bottom; }
		inline float GetTop() const { return m_Top; }

		inline CameraProjectionType GetProjectionType() const { return m_ProjectionType; }

	private:
		// View properties
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f }; // Pitch, Yaw, Roll
		glm::vec3 m_Target = { 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
		mutable glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		mutable bool m_ViewMatrixDirty = true;

		// Projection properties
		CameraProjectionType m_ProjectionType = CameraProjectionType::Perspective;

		// Perspective
		float m_FOV = 45.0f;
		float m_AspectRatio = 16.0f / 9.0f;
		float m_NearPlane = 0.1f;
		float m_FarPlane = 1000.0f;

		// Orthographic
		float m_Left = -1.0f;
		float m_Right = 1.0f;
		float m_Bottom = -1.0f;
		float m_Top = 1.0f;

		mutable glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		mutable bool m_ProjectionMatrixDirty = true;
	};

}
