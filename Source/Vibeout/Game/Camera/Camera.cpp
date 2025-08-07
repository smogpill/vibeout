// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Camera.h"

glm::dmat4 Camera::GetViewMatrix() const
{
	return Inverse(_node.GetGlobalTransform()).To_dmat4();
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(glm::radians(_verticalFOV), _aspectRatio, _nearClipDist, _farClipDist);
}
