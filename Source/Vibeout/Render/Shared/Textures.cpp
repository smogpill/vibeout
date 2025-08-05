// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Textures.h"

Textures::Textures(Renderer& renderer, bool& result)
	: _renderer(renderer)
{
	result = Init();
}

Textures::~Textures()
{
}

bool Textures::Init()
{
	return true;
}
