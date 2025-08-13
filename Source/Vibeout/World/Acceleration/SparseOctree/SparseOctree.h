// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class SparseOctree
{
public:
	SparseOctree(std::vector<uint32>&& data);

	static const uint32 s_fullInteriorNodePtr = uint32(-1);

private:
	std::vector<uint32> _data;
};
