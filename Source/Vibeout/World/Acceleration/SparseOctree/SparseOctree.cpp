// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "SparseOctree.h"

SparseOctree::SparseOctree(std::vector<uint32>&& data)
	: _data(std::move(data))
{
}
