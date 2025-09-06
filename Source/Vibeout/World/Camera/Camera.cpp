// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Camera.h"

void Camera::Rotate(float yaw, float pitch)
{
    _yaw -= yaw * _sensitivity;
    _pitch -= pitch * _sensitivity;
    const float pitchLimit = glm::half_pi<float>() * 0.99f;
    _pitch = glm::clamp(_pitch, -pitchLimit, pitchLimit);
    Transform transform = _node.GetGlobalTransform();
    transform.Rotation() = glm::quat(glm::vec3(_pitch, _yaw, 0.0f));
    _node.SetGlobalTransform(transform);
}

void Camera::SetTranslation(const glm::dvec3& pos)
{
    Transform transform = _node.GetGlobalTransform();
    transform.Translation() = pos;
    return _node.SetGlobalTransform(transform);
}

void Camera::SetAspectRatio(float aspectRatio)
{
    _aspectRatio = aspectRatio;
}

glm::dmat4 Camera::GetViewMatrix() const
{
	return Inverse(_node.GetGlobalTransform()).To_dmat4();
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(_verticalFOV), _aspectRatio, _nearClipDist, _farClipDist);
}
