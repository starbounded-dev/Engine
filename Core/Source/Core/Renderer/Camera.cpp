#include "Camera.h"

#include "Core/Debug/Profiler.h"

namespace Core::Renderer {

	Camera::Camera()
	{
		RecalculateProjectionMatrix();
		RecalculateViewMatrix();
	}

	Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_ProjectionType(CameraProjectionType::Perspective)
		, m_FOV(fov)
		, m_AspectRatio(aspectRatio)
		, m_NearPlane(nearPlane)
		, m_FarPlane(farPlane)
	{
		RecalculateProjectionMatrix();
		RecalculateViewMatrix();
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: m_ProjectionType(CameraProjectionType::Orthographic)
		, m_Left(left)
		, m_Right(right)
		, m_Bottom(bottom)
		, m_Top(top)
		, m_NearPlane(nearPlane)
		, m_FarPlane(farPlane)
	{
		RecalculateProjectionMatrix();
		RecalculateViewMatrix();
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		m_Position = position;
		m_ViewMatrixDirty = true;
	}

	void Camera::SetRotation(const glm::vec3& rotation)
	{
		m_Rotation = rotation;
		m_ViewMatrixDirty = true;
	}

	void Camera::SetTarget(const glm::vec3& target)
	{
		m_Target = target;
		m_ViewMatrixDirty = true;
	}

	void Camera::SetUp(const glm::vec3& up)
	{
		m_Up = up;
		m_ViewMatrixDirty = true;
	}

	void Camera::SetPerspective(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		m_ProjectionType = CameraProjectionType::Perspective;
		m_FOV = fov;
		m_AspectRatio = aspectRatio;
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_ProjectionMatrixDirty = true;
	}

	void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		m_ProjectionType = CameraProjectionType::Orthographic;
		m_Left = left;
		m_Right = right;
		m_Bottom = bottom;
		m_Top = top;
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		m_ProjectionMatrixDirty = true;
	}

	void Camera::SetProjectionType(CameraProjectionType type)
	{
		m_ProjectionType = type;
		m_ProjectionMatrixDirty = true;
	}

	const glm::mat4& Camera::GetViewMatrix() const
	{
		if (m_ViewMatrixDirty)
			RecalculateViewMatrix();
		return m_ViewMatrix;
	}

	const glm::mat4& Camera::GetProjectionMatrix() const
	{
		if (m_ProjectionMatrixDirty)
			RecalculateProjectionMatrix();
		return m_ProjectionMatrix;
	}

	glm::mat4 Camera::GetViewProjectionMatrix() const
	{
		return GetProjectionMatrix() * GetViewMatrix();
	}

	void Camera::RecalculateViewMatrix() const
	{
		PROFILE_FUNC();

		// If using look-at mode (target is set and different from position)
		if (glm::length(m_Target - m_Position) > 0.0001f)
		{
			m_ViewMatrix = glm::lookAt(m_Position, m_Target, m_Up);
		}
		else
		{
			// Calculate view matrix from position and rotation (Euler angles)
			glm::mat4 rotation = glm::mat4(1.0f);
			rotation = glm::rotate(rotation, m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
			rotation = glm::rotate(rotation, m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
			rotation = glm::rotate(rotation, m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll

			glm::vec3 forward = glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
			glm::vec3 right = glm::vec3(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
			glm::vec3 up = glm::cross(right, forward);

			m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, up);
		}

		m_ViewMatrixDirty = false;
	}

	void Camera::RecalculateProjectionMatrix() const
	{
		PROFILE_FUNC();

		if (m_ProjectionType == CameraProjectionType::Perspective)
		{
			m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
		}
		else
		{
			m_ProjectionMatrix = glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearPlane, m_FarPlane);
		}

		m_ProjectionMatrixDirty = false;
	}

}
