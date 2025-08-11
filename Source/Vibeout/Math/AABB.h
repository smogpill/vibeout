// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class AABB
{
public:
	AABB() = default;
	AABB(const glm::vec3& min, const glm::vec3& max) : _min(min), _max(max) {}

	void Clear() { *this = AABB(); }
	auto Min() -> glm::vec3& { return _min; }
	auto Min() const -> const glm::vec3& { return _min; }
	auto Max() -> glm::vec3& { return _max; }
	auto Max() const -> const glm::vec3& { return _max; }
	auto GetSize() const { return _max - _min; }
	bool IsValid() const { return glm::all(glm::greaterThanEqual(_max, _min)); }

	static auto GetInvalid() -> const AABB& { static const AABB invalid; return invalid; }

private:
	glm::vec3 _min = glm::vec3(1e20f);
	glm::vec3 _max = -glm::vec3(1e20f);
};
