// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Math/Transform.h"
#include "Vibeout/World/Node/Node.h"

class Camera
{
public:
	void Rotate(float yaw, float pitch);
	void SetTranslation(const glm::dvec3& pos);
	void SetAspectRatio(float aspectRatio);
	auto GetNode() -> Node& { return _node; }
	auto GetNode() const -> const Node& { return _node; }
	auto GetViewMatrix() const -> glm::dmat4;
	auto GetProjectionMatrix() const -> glm::mat4;
	/// In degrees
	auto GetVerticalFOV() const -> float { return _verticalFOV; }

private:
	// Transform/Motion
	Node _node;
	float _yaw = 0.0f;
	float _pitch = 0.0f;
	float _roll = 0.0f;
	float _sensitivity = 0.01f;

	float _verticalFOV = 70.0f;
	float _nearClipDist = 0.01f;
	float _farClipDist = 10000.0f;
	float _aspectRatio = 1024.0f / 768.0f;
};
