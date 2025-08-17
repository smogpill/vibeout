// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class SparseOctree
{
public:
	SparseOctree(std::vector<uint32>&& data);

	static const uint32 s_interiorNodePtr = uint32(-1);
	auto GetData() const -> const void* { return _data.data(); }
	auto GetDataSize() const -> uint32 { return (uint32)_data.size() * sizeof(uint32); }

private:
	std::vector<uint32> _data;
};
