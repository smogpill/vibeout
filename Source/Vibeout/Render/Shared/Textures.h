// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Renderer;

class Textures
{
public:
	Textures(Renderer& renderer, bool& result);
	~Textures();

private:
	bool Init();

	Renderer& _renderer;
};
