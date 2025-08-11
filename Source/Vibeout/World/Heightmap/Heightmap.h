// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Heightmap
{
public:
	Heightmap(const char* path, bool& result);
	auto Size() -> glm::ivec3& { return _size; }
	auto Size() const -> const glm::ivec3& { return _size; }
	auto Data() const -> const std::vector<uint16>& { return _data; }

private:
	bool Init(const char* path);

	std::vector<uint16> _data;
	glm::ivec3 _size;
};
