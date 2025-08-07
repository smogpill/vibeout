// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Transform.h"
#include "Vibeout/World/Node.h"

class Camera
{
public:
	void OnFixedUpdate()
	{

	}
	void OnUpdate()
	{

	}

	const Node& GetNode() const { return _node; }
	glm::dmat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	/// In degrees
	float GetVerticalFOV() const { return _verticalFOV; }

private:
	Node _node;
	float _verticalFOV = 70.0f;
	float _nearClipDist = 0.01f;
	float _farClipDist = 10000.0f;
	float _aspectRatio = 1024.0f / 768.0f;
};
