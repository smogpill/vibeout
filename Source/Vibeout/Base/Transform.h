// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Transform
{
public:
	Transform() = default;
	Transform(const glm::dquat& rotation, const glm::dvec3& translation) : _rotation(rotation), _translation(translation) {}

	glm::dquat& Rotation() { return _rotation; }
	const glm::dquat& Rotation() const { return _rotation; }
	glm::dvec3& Translation() { return _translation; }
	const glm::dvec3& Translation() const { return _translation; }
	Transform operator*(const Transform& other);
	glm::dmat4 To_dmat4() const;

private:
	glm::dquat _rotation = glm::identity<glm::dquat>();
	glm::dvec3 _translation = glm::dvec3(0);
};

inline Transform Transform::operator*(const Transform& other)
{
	Transform result;
	result._rotation = _rotation * other._rotation;
	result._translation = _translation + _rotation * other._translation;
	return result;
}

inline glm::dmat4 Transform::To_dmat4() const
{
	return glm::translate(glm::dmat4(1.0f), _translation) * glm::mat4_cast(_rotation);
}

inline Transform Inverse(const Transform& t)
{
	Transform inv;
    inv.Rotation() = glm::conjugate(t.Rotation());
    inv.Translation() = -(inv.Rotation() * t.Translation());
    return inv;
}
