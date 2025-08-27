// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

bool ReadBinaryFile(const std::string& path, std::vector<char>& buffer);
template <class T>
constexpr T AlignUp(const T val, const T alignment)
{
	return (val + (alignment - 1)) & ~(alignment - 1);
}
