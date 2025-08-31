// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Material;
class ResourceLoader;

class Model : public Resource
{
	using Base = Resource;
public:
	~Model();

	bool OnLoad(ResourceLoader& loader) override;
	
private:
	struct Vertex
	{
		float _pos[3];
		float _normal[3];
		float _uvs[2];
	};
	struct Shape
	{
		std::vector<uint32> _indices;
	};

	std::vector<Vertex> _vertices;
	std::vector<Shape> _shapes;
	std::vector<std::shared_ptr<Material>> _materials;
};
