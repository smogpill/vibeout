// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Asset
{
public:
	virtual ~Asset() = default;

	virtual void Load(const char* path);
};
