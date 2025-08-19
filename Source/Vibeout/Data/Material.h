// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Texture;

class Material
{
public:
	std::shared_ptr<Texture> _color;
	std::shared_ptr<Texture> _normal;
	std::shared_ptr<Texture> _metalness;
	std::shared_ptr<Texture> _roughness;
	std::shared_ptr<Texture> _emissive;
};
