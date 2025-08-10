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

void Camera::OnUpdate(float deltaTime)
{
    Transform global = _node.GetGlobalTransform();
    glm::dvec3& pos = global.Translation();
    const bool* keys = SDL_GetKeyboardState(nullptr);
    glm::vec3 localMoveIntent(0.0f);
    if (keys[SDL_SCANCODE_W]) localMoveIntent[2] -= 1.0f;
    if (keys[SDL_SCANCODE_S]) localMoveIntent[2] += 1.0f;
    if (keys[SDL_SCANCODE_A]) localMoveIntent[0] -= 1.0f;
    if (keys[SDL_SCANCODE_D]) localMoveIntent[0] += 1.0f;
    const glm::mat3 globalRotMat = glm::mat4(global.To_dmat4());
    const glm::vec3 globalMoveIntent = globalRotMat * localMoveIntent;
    pos += globalMoveIntent * 0.01f;

    global.Rotation() = glm::quat(glm::vec3(_pitch, _yaw, 0.0f));
    _node.SetGlobalTransform(global);
}

glm::dmat4 Camera::GetViewMatrix() const
{
	return Inverse(_node.GetGlobalTransform()).To_dmat4();
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(_verticalFOV), _aspectRatio, _nearClipDist, _farClipDist);
}
